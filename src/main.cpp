/*
    ChibiOS - Copyright (C) 2006..2016 Giovanni Di Sirio
    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at
        http://www.apache.org/licenses/LICENSE-2.0
    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#include "ch.hpp"
#include "hal.h"
#include "node.hpp"

//#include "shell.h"
//#include "chprintf.h"

#include "hardware.hpp"

#include <config/config.hpp>

#include <bootloader_interface/bootloader_interface.hpp>
#include "os.hpp"


//#define SHELL_WA_SIZE   THD_WORKING_AREA_SIZE(2048)
//
//static void cmd_enter_recovery(BaseSequentialStream *chp, int argc, char *argv[]) {
//}
//
//uint8_t char_to_int(char c) {
//    return c - '0';
//}




//static const ShellCommand commands[] = {
//    {"recovery", cmd_enter_recovery},
//    {NULL, NULL}
//};

/*
 * Shell history buffer
 */
//char history_buffer[SHELL_MAX_HIST_BUFF];

/*
 * Shell completion buffer
 */
//char *completion_buffer[SHELL_MAX_COMPLETIONS];
//
//static const ShellConfig shell_cfg1 = {
//    (BaseSequentialStream *)&SD1,
//    commands,
//    history_buffer,
//    sizeof(history_buffer),
//    completion_buffer
//};

auto onFirmwareUpdateRequestedFromUAVCAN(
    const uavcan::ReceivedDataStructure<uavcan::protocol::file::BeginFirmwareUpdate::Request>& request)
{
    /*
     * Checking preconditions
     */
    static bool already_in_progress = false;

    const std::uint8_t source_node_id =
        ((request.source_node_id > 0) &&
         (request.source_node_id <= uavcan::NodeID::Max) &&
         uavcan::NodeID(request.source_node_id).isUnicast()) ?
            request.source_node_id :
            request.getSrcNodeID().get();

//    os::lowsyslog("UAVCAN firmware update request from %d, source %d, path '%s'\n",
//                  request.getSrcNodeID().get(),
//                  source_node_id,
//                  request.image_file_remote_path.path.c_str());

    if (already_in_progress)
    {
//        os::lowsyslog("UAVCAN firmware update is already in progress, rejecting\n");
        return uavcan::protocol::file::BeginFirmwareUpdate::Response::ERROR_IN_PROGRESS;
    }

    /*
     * Initializing the app shared structure with proper arguments
     */
    //Node::Lock locker;

    bootloader_interface::AppShared shared;
    shared.can_bus_speed = Node::getCANBitRate();
    shared.uavcan_node_id = Node::getNode().getNodeID().get();
    shared.uavcan_fw_server_node_id = source_node_id;
    shared.stay_in_bootloader = true;

    std::strncpy(static_cast<char*>(&shared.uavcan_file_name[0]),       // This is really messy
                 request.image_file_remote_path.path.c_str(),
                 shared.UAVCANFileNameMaxLength);
    shared.uavcan_file_name[shared.UAVCANFileNameMaxLength - 1] = '\0';

    static_assert(request.image_file_remote_path.path.MaxSize < shared.UAVCANFileNameMaxLength, "Err...");

//    os::lowsyslog("Bootloader args: CAN bus bitrate: %u, local node ID: %d\n",
//                  unsigned(shared.can_bus_speed), shared.uavcan_node_id);

    /*
     * Commiting everything
     */
    bootloader_interface::writeSharedStruct(shared);

    NVIC_SystemReset();

    already_in_progress = true;

    os::lowsyslog("UAVCAN firmware update initiated\n");
    return uavcan::protocol::file::BeginFirmwareUpdate::Response::ERROR_OK;
}


int main(void) {
    os::watchdog::Timer wdt = Hardware::init();

    const auto fw_version = bootloader_interface::getFirmwareVersion();

    const auto app_shared_read_result =
            bootloader_interface::readAndInvalidateSharedStruct();
    const auto& app_shared = app_shared_read_result.first;
    const auto app_shared_available = app_shared_read_result.second;

    Node::init(1000000, 11, fw_version.major, fw_version.minor,
            fw_version.vcs_commit, fw_version.image_crc64we,
            &onFirmwareUpdateRequestedFromUAVCAN);


//   thread_t *shelltp = chThdCreateFromHeap(NULL, SHELL_WA_SIZE,
//            "shell", NORMALPRIO + 1, shellThread, (void *)&shell_cfg1);

  while (true) {
      wdt.reset();
      chThdSleepMilliseconds(500);
  }
}
