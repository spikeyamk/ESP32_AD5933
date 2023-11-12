#pragma once

#include <array>
#include <cstdint>

namespace MagicPackets {
	extern const std::array<uint8_t, 20> debug_start;
	extern const std::array<uint8_t, 20> dump_all_registers;
	extern const std::array<uint8_t, 20> program_all_registers;
	extern const std::array<uint8_t, 20> control_HB_command;
	extern const std::array<uint8_t, 20> debug_end;
	extern const std::array<std::array<uint8_t, 20>, 5> magic_packets;
}
