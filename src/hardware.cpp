#include "ch.hpp"
#include "hal.h"

#include <hardware.hpp>
#include <cstring>
#include <math.h>

#include <config/config_storage_flash.hpp>
#include <config/config.hpp>

namespace Hardware {

    UUID readUniqueID()
    {
        UUID out_bytes;
        memcpy(out_bytes.data(), reinterpret_cast<const void*>(0x1FFFF7E8), std::tuple_size<UUID>::value);
        return out_bytes;
    }

    void setStatusLed(bool on) {
//        palWritePad(PORT_LED_RED, GPIO_LED_RED, on);
    }

    void setCANLed(bool on) {
//        palWritePad(PORT_LED_GREEN, GPIO_LED_GREEN, on);
    }

    static void* const ConfigStorageAddress = reinterpret_cast<void*>(0x08000000 + (256 * 1024) - 1024);
    constexpr unsigned ConfigStorageSize = 1024;

    os::watchdog::Timer init() {
        halInit();
        chSysInit();

        os::watchdog::init();
        os::watchdog::Timer wdt;
        wdt.startMSec(5000);
        wdt.reset();

        sdStart(&SD1, NULL);

        static os::stm32::ConfigStorageBackend config_storage_backend(ConfigStorageAddress, ConfigStorageSize);
        const int config_init_res = os::config::init(&config_storage_backend);

        return wdt;
    }
}