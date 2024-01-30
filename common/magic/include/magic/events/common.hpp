#pragma once

#include <array>
#include <cstdint>
#include <cstddef>

namespace Magic {
    constexpr size_t MTU = 20;
    using T_MaxPacket = std::array<uint8_t, MTU>;
    using T_MaxDataSlice = std::array<uint8_t, MTU - 1>;
}