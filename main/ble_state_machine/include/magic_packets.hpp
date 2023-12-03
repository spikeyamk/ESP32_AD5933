#pragma once

#include <array>
#include <optional>
#include <bitset>
#include <vector>
#include <cstdint>

namespace MagicPackets {
	using MagicPacket_T = std::array<uint8_t, 20>;
	namespace Debug {
		namespace Command {
			extern const MagicPacket_T start;
			extern const MagicPacket_T dump_all_registers;
			extern const MagicPacket_T program_all_registers;
			extern const MagicPacket_T control_HB_command;
			extern const MagicPacket_T end;
			extern const std::array<MagicPacket_T, 5> chunks;
		}
	}
	namespace FrequencySweep {
		namespace Command {
			extern const MagicPacket_T configure;
			extern const MagicPacket_T initialize_with_start_freq;
			extern const MagicPacket_T start;
			extern const MagicPacket_T check_for_data_valid;
			extern const MagicPacket_T read_data_valid_value;
			extern const MagicPacket_T repeat_freq;
			extern const MagicPacket_T check_for_sweep_complete;
			extern const MagicPacket_T increment_frequency;
			extern const MagicPacket_T repeat_frequency_sweep;
			extern const MagicPacket_T stop_frequency_sweep;
			extern const std::array<MagicPacket_T, 10> chunks;
		}

		namespace Condition {
			extern const MagicPacket_T data_valid_false;
			extern const MagicPacket_T data_valid_true;
			extern const MagicPacket_T sweep_complete_false;
			extern const MagicPacket_T sweep_complete_true;
			extern const std::array<MagicPacket_T, 4> chunks;
		}
	}
	extern const std::array<const MagicPacket_T*, 15> all_packets;
	std::optional<size_t> find_footer_start_index(const MagicPacket_T& packet);
	std::optional<const MagicPacket_T*> get_magic_packet_pointer(const MagicPacket_T &packet);
	std::optional<std::vector<std::bitset<8>>> get_packet_data(const MagicPackets::MagicPacket_T &in_raw_packet);
	std::vector<uint8_t> get_raw_packet_data(const MagicPackets::MagicPacket_T &in_raw_packet, const size_t footer_start_index);

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

