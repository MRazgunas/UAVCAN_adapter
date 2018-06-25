#pragma once

#include <array>
#include <cstdint>

#include <os.hpp>

namespace Hardware {

    typedef std::array<std::uint8_t, 12> UUID;
    UUID readUniqueID();

    void setStatusLed(bool on);
    void setCANLed(bool on);

    void restart_i2c();

    bool isRebootRequested();
    void requestReboot();

    os::watchdog::Timer init();
}
