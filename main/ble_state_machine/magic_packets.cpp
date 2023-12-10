#include "include/magic_packets.hpp"

#include <array>
#include <optional>
#include <cstdint>
#include <algorithm>
#include <vector>
#include <bitset>
#include <span>

namespace MagicPackets {
	namespace Debug {
		namespace Command {
			constinit const MagicPacket_T start {
				0x7d, 0x68, 0x97, 0xda, 0xf4, 0x5c, 0x2b, 0xd5, 
				0x49, 0x26, 0x55, 0x83, 0xa2, 0x38, 0xfa, 0x54, 
				0xb9, 0x5c, 0x14, 0x14,
			};
			constinit const MagicPacket_T dump_all_registers {
				// 12 8-bit Read/Write Registers placeholder
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00,

				// 8-bit Special STATUS Register placheholder
				0x00,

				// 6 8-bit Read-only Registers placheholder
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 

				0xFF,
			};
			constinit const MagicPacket_T program_all_registers {
				// 12 8-bit Read/Write Registers placeholder
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 

				0xFF,

				0xf2, 0xb7, 0xd0, 0x23,
				0xee, 0xae, 0x97, 
			};
			constinit const MagicPacket_T control_HB_command {
				0b0000'0000, // 4-bit placeholder

				0xFF,

				0xe8, 0x1a, 0x82, 0x05, 0xb5, 0xcf, 0x8a, 0x0b, 
				0x0e, 0xbf, 0x45, 0x3c, 0x40, 0xa7, 0x20, 0x83, 
				0x65, 0x36, 
			};
			constinit const MagicPacket_T end {
				0x31, 0x5a, 0xe0, 0x1f, 0x76, 0xc4, 0x6b, 0x1c, 
				0xab, 0x57, 0xa7, 0x93, 0x44, 0xb3, 0x4d, 0x39, 
				0x52, 0xf2, 0xdb, 0x11,
			};
			/*
			constinit const std::array<std::array<uint8_t, 23>, 5> chunks { 
				start,
				dump_all_registers, program_all_registers, control_HB_command,
				end,
			};
			*/
		}
	}

	namespace FrequencySweep {
		namespace Command {
			constinit const MagicPacket_T configure {
				// 12 8-bit Read/Write Registers placeholder
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 

				0xFF,

				0xa8, 0xca, 0x9e, 0x91, 0x8e, 0xd3, 0x88, 
			};

			constinit const MagicPacket_T initialize_with_start_freq {
				0x9f, 0x0b, 0x0a, 0x00, 0x55, 0xe6, 0x2b, 0xe7, 
				0x6f, 0x13, 0x94, 0x39, 0x85, 0x0e, 0xf4, 0x18, 
				0x25, 0x23, 0x69, 0x36,
			};

			constinit const MagicPacket_T start {
				0x91, 0x12, 0x0c, 0xbc, 0x10, 0x8c, 0x75, 0xcb, 
				0x9f, 0x83, 0x91, 0x8c, 0x01, 0x7f, 0x43, 0x24, 
				0x31, 0x0a, 0x6a, 0x02,
			};
			constinit const MagicPacket_T check_for_data_valid {
				0b0000'0000, // bool placeholder

				0xFF,

				0xc5, 0x00, 0x41, 0x6d, 0xf4, 0x3f, 0xc0, 0xcc, 
				0x4a, 0x9d, 0xe1, 0xdc, 0xf7, 0x16, 0x47, 0xfb,
				0x4a, 0xa6,
			};
			constinit const MagicPacket_T read_data_valid_value {
				0x00, 0x00, // REAL_DATA (RAW)
				0x00, 0x00, // IMAG_DATA (RAW)
				0x00, 0x00, 0x00, 0x00, // REAL_DATA (float)
				0x00, 0x00, 0x00, 0x00, // IMAG_DATA (float)

				0xFF,

				0xb7, 0x02, 0x82, 0xf2, 0x19, 0xde, 0x45,
			};
			constinit const MagicPacket_T repeat_freq {
				0x3f, 0xb7, 0x4d, 0xe0, 0xd8, 0x05, 0x71, 0x02, 
				0x87, 0xc9, 0xbb, 0xbb, 0x75, 0x35, 0xa5, 0x4c, 
				0x60, 0x04, 0x00, 0x9f,
			};
			constinit const MagicPacket_T check_for_sweep_complete {
				0b0000'0000, // bool placeholder

				0xFF,

				0x5a, 0xec, 0xce, 0xf4, 0x1c, 0x80, 0xa3, 0xd2, 
				0x31, 0x39, 0xd5, 0xdf, 0x3f, 0xb1, 0x46, 0x8f, 
				0x7c, 0x2b, 	
			};
			constinit const MagicPacket_T increment_frequency {
				0x4a, 0x7c, 0x2e, 0x99, 0xe7, 0x20, 0x07, 0xa5, 
				0x14, 0xe6, 0xdc, 0x49, 0x24, 0x94, 0x28, 0xfd, 
				0x48, 0x5c, 0xcf, 0x72,
			};
			constinit const MagicPacket_T repeat_frequency_sweep {
				0xf3, 0x52, 0xb0, 0x32, 0x4a, 0x5c, 0x60, 0xb1, 
				0xf5, 0x52, 0x35, 0xbb, 0x95, 0x67, 0x1e, 0xb5, 
				0x3e, 0x67, 0xd4, 0xae,
			};
			constinit const MagicPacket_T stop_frequency_sweep {
				0xa8, 0xed, 0x4b, 0x09, 0xe2, 0xc7, 0x7f, 0x13, 
				0xac, 0xb9, 0xae, 0xbc, 0x5c, 0x76, 0x94, 0x05, 
				0x45, 0x02, 0x59, 0x06,
			};
			/*
			constinit const std::array<std::array<uint8_t, 23>, 10> chunks {
				configure, initialize_with_start_freq, start,
				check_for_data_valid, read_data_valid_value, repeat_freq,
				check_for_sweep_complete, increment_frequency, repeat_frequency_sweep,
				stop_frequency_sweep
			};
			*/
		}


		namespace Condition {
			/*
			constinit const std::array<uint8_t, 23> data_valid_true {
				0xe3, 0xb1, 0x88, 0x52, 0x08, 0x52, 0xe7, 0xee, 
				0xb4, 0xf8, 0x4b, 0x9d, 0x10, 0x55, 0x19, 0x36, 
				0x88, 0x99, 0x1a, 0xb5, 0x10, 0x81, 0x96, 
			};
			constinit const std::array<uint8_t, 23> sweep_complete_false {
				0x1f, 0x0a, 0x2d, 0x11, 0xde, 0x1e, 0xa5, 0x40, 
				0x92, 0x0b, 0x7e, 0x03, 0xad, 0x8e, 0xd3, 0xea, 
				0xba, 0x80, 0xc3, 0x8d, 0x76, 0x7f, 0x84, 
			};
			constinit const std::array<uint8_t, 23> sweep_complete_true {
				0x30, 0x01, 0xed, 0x78, 0xee, 0xa7, 0xd5, 0xfe, 
				0x48, 0x7c, 0xfd, 0x49, 0x83, 0xaa, 0x8f, 0x51, 
				0xac, 0x6a, 0x51, 0xa9, 0x03, 0xe8, 0x93, 
			};
			constinit const std::array<std::array<uint8_t, 23>, 4> chunks {
				data_valid_false, data_valid_true, sweep_complete_false,
				sweep_complete_true
			};
			*/
		}
	}
	using namespace Debug::Command;
	using namespace FrequencySweep::Command;

	const std::array<const MagicPacket_T*, 15> all_packets { 
		&Debug::Command::start,
		&Debug::Command::dump_all_registers, &Debug::Command::program_all_registers, &Debug::Command::control_HB_command,
		&Debug::Command::end,

		&FrequencySweep::Command::configure, &FrequencySweep::Command::initialize_with_start_freq,

		&FrequencySweep::Command::start,

		&FrequencySweep::Command::check_for_data_valid, &FrequencySweep::Command::read_data_valid_value, &FrequencySweep::Command::repeat_freq,
		&FrequencySweep::Command::check_for_sweep_complete, &FrequencySweep::Command::increment_frequency, &FrequencySweep::Command::repeat_frequency_sweep,
		&FrequencySweep::Command::stop_frequency_sweep
	};

	namespace Header {
		/* Remains unused for the time being
		constinit const std::array<uint8_t, 23> command_for_server {
			0xee, 0x6c, 0xdf, 0xd3, 0x0f, 0x47, 0xe2, 0x4e, 
			0x73, 0xb5, 0x90, 0xe9, 0xfa, 0x5f, 0x32, 0x49, 
			0xc5, 0xae, 0x36, 0xbc, 0xe5, 0xda, 0xe9, 
		};
		constinit const std::array<uint8_t, 23> data_for_server {
			0x15, 0xf2, 0xc6, 0x2d, 0xeb, 0xfa, 0x81, 0xff, 
			0x9a, 0x44, 0x93, 0xf0, 0xe5, 0xb3, 0x75, 0xe1, 
			0xc8, 0xb9, 0xae, 0xb6, 0xa2, 0x6b, 0x79, 
		};
		constinit const std::array<uint8_t, 23> data_for_client {
			0x98, 0x41, 0xb9, 0x18, 0x79, 0x31, 0x37, 0xe2, 
			0x83, 0xff, 0xd2, 0x43, 0xd2, 0x8b, 0xb0, 0xf7, 
			0x73, 0xf5, 0x31, 0x43, 0xce, 0x0e, 0x3e, 
		};
		constinit const std::array<uint8_t, 23> data_valid {
			0x62, 0xc7, 0x7e, 0x13, 0xdd, 0xf6, 0x95, 0x19, 
			0xf8, 0xe4, 0x4d, 0x02, 0x8a, 0x34, 0xa4, 0xb9, 
			0x6e, 0x0c, 0xee, 0xfd, 0x43, 0xcf, 0xd3, 
		};
		constinit const std::array<std::array<uint8_t, 23>, 4> chunks {
			command_for_server, data_for_server, data_for_client,
			data_valid
		};
		*/
	}
	
	namespace Logging {
		// Placeholder
	}

}

namespace MagicPackets {
	static constexpr uint8_t magic_footer_starter = 0xFF;
	std::optional<size_t> find_footer_start_index(const MagicPackets::MagicPacket_T& raw_packet) {
		const auto it = std::find(std::rbegin(raw_packet), std::rend(raw_packet), magic_footer_starter);

		if(it != std::rend(raw_packet)) {
			return std::distance(it, std::rend(raw_packet)) - 1;
		}

		return std::nullopt;
	}

	MagicPackets::MagicPacket_T get_raw_packet_footer(const MagicPackets::MagicPacket_T &in_raw_packet, const size_t footer_start_index) {
		MagicPackets::MagicPacket_T raw_packet = in_raw_packet;
		for(size_t i = 0; i < footer_start_index; i++) {
			raw_packet[i] = 0;
		}
		return raw_packet;
	}

	std::vector<uint8_t> get_raw_packet_data(const MagicPackets::MagicPacket_T &in_raw_packet, const size_t footer_start_index) {
		return std::vector<uint8_t>{ in_raw_packet.begin(), in_raw_packet.begin() + footer_start_index };
	}

	std::optional<std::vector<std::bitset<8>>> get_packet_data(const MagicPackets::MagicPacket_T &in_raw_packet) {
		const std::optional<size_t> footer_start_index = find_footer_start_index(in_raw_packet);
		if(footer_start_index.has_value() == false) {
			return std::nullopt;
		}
		const std::vector<uint8_t> raw_packet_data { get_raw_packet_data(in_raw_packet, footer_start_index.value()) };
		std::vector<std::bitset<8>> ret;
		for(const auto i: raw_packet_data) {
			ret.push_back(std::bitset<8> { i });
		}
		return ret;
	}
}

namespace MagicPackets {
	std::optional<const MagicPacket_T*> get_magic_packet_pointer(const MagicPacket_T &raw_packet) {
		const std::optional<size_t> footer_start_index = find_footer_start_index(raw_packet);
		if(footer_start_index.has_value()) {
			const MagicPacket_T packet_with_only_footer { get_raw_packet_footer(raw_packet, footer_start_index.value()) };
			for(size_t i = 0; i < MagicPackets::all_packets.size(); i++) {
				if(packet_with_only_footer == *(MagicPackets::all_packets[i])) {
					return MagicPackets::all_packets[i];
				}
			}
		} else {
			for(size_t i = 0; i < MagicPackets::all_packets.size(); i++) {
				if(raw_packet == *(MagicPackets::all_packets[i])) {
					return MagicPackets::all_packets[i];
				}
			}
		}
		return std::nullopt;
	}
}
