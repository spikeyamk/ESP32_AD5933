#pragma once

#include <array>
#include <optional>
#include <bitset>
#include <vector>
#include <cstdint>

namespace MagicPackets {
	namespace Debug {
		namespace Command {
			extern const std::array<uint8_t, 23> start;
			extern const std::array<uint8_t, 23> dump_all_registers;
			extern const std::array<uint8_t, 23> program_all_registers;
			extern const std::array<uint8_t, 23> control_HB_command;
			extern const std::array<uint8_t, 23> end;
			extern const std::array<std::array<uint8_t, 23>, 5> chunks;
		}
	}
	namespace FrequencySweep {
		namespace Command {
			extern const std::array<uint8_t, 23> configure;
			extern const std::array<uint8_t, 23> initialize_with_start_freq;
			extern const std::array<uint8_t, 23> start;
			extern const std::array<uint8_t, 23> check_for_data_valid;
			extern const std::array<uint8_t, 23> read_data_valid_value;
			extern const std::array<uint8_t, 23> repeat_freq;
			extern const std::array<uint8_t, 23> check_for_sweep_complete;
			extern const std::array<uint8_t, 23> increment_frequency;
			extern const std::array<uint8_t, 23> repeat_frequency_sweep;
			extern const std::array<uint8_t, 23> stop_frequency_sweep;
			extern const std::array<std::array<uint8_t, 23>, 10> chunks;
		}

		namespace Condition {
			extern const std::array<uint8_t, 23> data_valid_false;
			extern const std::array<uint8_t, 23> data_valid_true;
			extern const std::array<uint8_t, 23> sweep_complete_false;
			extern const std::array<uint8_t, 23> sweep_complete_true;
			extern const std::array<std::array<uint8_t, 23>, 4> chunks;
		}
	}
	extern const std::array<const std::array<uint8_t, 23>*, 15> all_packets;
	std::optional<size_t> find_footer_start_index(const std::array<uint8_t, 23>& packet);
	std::optional<const std::array<uint8_t, 23>*> get_magic_packet_pointer(const std::array<uint8_t, 23> &packet);
	std::optional<std::vector<std::bitset<8>>> get_packet_data(const std::array<uint8_t, 23> &in_raw_packet);
	std::vector<uint8_t> get_raw_packet_data(const std::array<uint8_t, 23> &in_raw_packet, const size_t footer_start_index);

	namespace Header {
		/* Remains unused for the time being
		extern const std::array<uint8_t, 23> command_for_server;
		extern const std::array<uint8_t, 23> data_for_server;
		extern const std::array<uint8_t, 23> data_for_client;
		extern const std::array<uint8_t, 23> data_valid;
		extern const std::array<std::array<uint8_t, 23>, 4> chunks;
		*/
	}
	namespace Logging {
		// Placeholder
	}
}

