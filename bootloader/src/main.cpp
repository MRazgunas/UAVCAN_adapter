#include <ch.hpp>
#include <hal.h>

#include <unistd.h>
#include <cstdlib>
#include <cassert>
#include <utility>

#include "os.hpp"
#include "flash_writer.hpp"
#include "bootloader.hpp"
#include "bootloader_app_interface.hpp"
#include "board.hpp"
#include "app_storage_backend.hpp"

#include "uavcan.hpp"

namespace app
{
namespace
{
constexpr unsigned WatchdogTimeoutMSec = 5000;


static inline std::pair<unsigned, unsigned> bootloaderStateToLEDOnOffDurationMSec(os::bootloader::State state)
{
    switch (state)
    {
    case os::bootloader::State::NoAppToBoot:
    {
        return {50, 50};
    }
    case os::bootloader::State::BootCancelled:
    {
        return {50, 950};
    }
    case os::bootloader::State::AppUpgradeInProgress:
    {
        return {500, 500};
    }
    case os::bootloader::State::BootDelay:
    case os::bootloader::State::ReadyToBoot:
    {
        return {0, 0};
    }
    }
    assert(false);
    return {0, 0};

}

}
}
/*

static THD_WORKING_AREA(waThread1, 128);
void Thread1(void) {
  chRegSetThreadName("blinker");

  while(1) {
    palClearPad(GPIOB, GPIOB_LED_RED);
    palSetPad(GPIOB, GPIOB_LED_GREEN);
    chThdSleepMilliseconds(500);
    palSetPad(GPIOB, GPIOB_LED_RED);
    palClearPad(GPIOB, GPIOB_LED_GREEN);
    chThdSleepMilliseconds(500);
  }
}*/

int main(void) {
  auto wdt = board::init(app::WatchdogTimeoutMSec);
  wdt.reset();
  
  chibios_rt::BaseThread::setPriority(LOWPRIO);

  board::AppStorageBackend backend;

  const ::systime_t bootloader_init_started_at = chVTGetSystemTimeX();
  static os::bootloader::Bootloader bl(backend, board::getFlashSize());

//  chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO, (tfunc_t)Thread1, NULL);

   {
        std::pair<bootloader_app_interface::AppShared, bool> app_shared = bootloader_app_interface::readAndErase();

        if (app_shared.second)
        {
            if (app_shared.first.stay_in_bootloader)
            {
                bl.cancelBoot();
            }

            // Ensuring the string is properly terminated:
            app_shared.first.uavcan_file_name[app_shared.first.UAVCANFileNameMaxLength - 1] = '\0';

           /* os::lowsyslog("AppShared: Wait %c; UAVCAN: %u bps %u/%u \"%s\"\n",
                          app_shared.first.stay_in_bootloader ? 'Y' : 'N',
                          unsigned(app_shared.first.can_bus_speed),
                          app_shared.first.uavcan_node_id,
                          app_shared.first.uavcan_fw_server_node_id,
                          &app_shared.first.uavcan_file_name[0]);
            */

            uavcan::init(bl,
                         app_shared.first.can_bus_speed,
                         app_shared.first.uavcan_node_id,
                         app_shared.first.uavcan_fw_server_node_id,
                         &app_shared.first.uavcan_file_name[0]);

        }

        else
        {
            uavcan::init(bl);
        }


    }

    /*
     * Main loop
     */
    while (!os::isRebootRequested())
    {
        wdt.reset();

        const auto bl_state = bl.getState();
        if (bl_state == os::bootloader::State::ReadyToBoot)
        {
            break;
        }

   //     board::setStatusLed(true);      // Always on
   //     const auto duration = app::bootloaderStateToLEDOnOffDurationMSec(bl_state);
   //     if (duration.first == 0 && duration.second == 0)
   //     {
            chThdSleepMilliseconds(100);
   //     }
   //     else
   //     {
   //         board::setCANLed(1, true);                  // Using CAN2 LED for state indication purposes
   //         chThdSleepMilliseconds(duration.first);
   //         board::setCANLed(1, false);
   //         chThdSleepMilliseconds(duration.second);
   //     }
    }

    if (os::isRebootRequested())
    {
//        os::lowsyslog("\nREBOOTING\n");
        chThdSleepMilliseconds(500);       // Providing some time for other components to react
        board::restart();
    }


  //  os::lowsyslog("\nBOOTING\n");
    os::requestReboot();         // Notifying other components that we're going down
    chThdSleepMilliseconds(500); // Providing some time for other components to react

    // Writing down the CAN bus parameters for the application, if known
    {
        const uavcan::Parameters p = uavcan::getParameters();

        bootloader_app_interface::AppShared as;
        as.can_bus_speed  = p.can_bus_bit_rate;
        as.uavcan_node_id = p.local_node_id;
        assert((as.can_bus_speed  <= 1000000) &&
               (as.uavcan_node_id <= 127));

        bootloader_app_interface::write(as);
    }

    wdt.reset();                 // The final reset, the application will have time to boot and init until next timeout
    board::bootApplication();

    return 0;
}

