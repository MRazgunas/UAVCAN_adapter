#pragma once

#include <array>
#include <cstdint>

#include <os.hpp>

namespace Hardware {

    typedef std::array<std::uint8_t, 12> UUID;
    UUID readUniqueID();

    void setStatusLed(bool on);
    void setCANLed(bool on);

    os::watchdog::Timer init();
}
