#pragma once

#include <array>
#include <optional>
#include <bitset>
#include <cstdint>
#include <span>
#include <functional>

namespace Magic {
	namespace Packets {
		using Packet_T = std::array<uint8_t, 20>;
		namespace Debug {
			constexpr Packet_T start {
				0x7d, 0x68, 0x97, 0xda, 0xf4, 0x5c, 0x2b, 0xd5, 
				0x49, 0x26, 0x55, 0x83, 0xa2, 0x38, 0xfa, 0x54, 
				0xb9, 0x5c, 0x14, 0x14,
			};
			constexpr Packet_T dump_all_registers {
				// 12 8-bit Read/Write Registers placeholder
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00,

				// 8-bit Special STATUS Register placheholder
				0x00,

				// 6 8-bit Read-only Registers placheholder
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 

				0xFF,
			};
			constexpr Packet_T program_all_registers {
				// 12 8-bit Read/Write Registers placeholder
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 

				0xFF,

				0xf2, 0xb7, 0xd0, 0x23,
				0xee, 0xae, 0x97, 
			};
			constexpr Packet_T control_HB_command {
				0b0000'0000, // 4-bit placeholder

				0xFF,

				0xe8, 0x1a, 0x82, 0x05, 0xb5, 0xcf, 0x8a, 0x0b, 
				0x0e, 0xbf, 0x45, 0x3c, 0x40, 0xa7, 0x20, 0x83, 
				0x65, 0x36, 
			};
			constexpr Packet_T end {
				0x31, 0x5a, 0xe0, 0x1f, 0x76, 0xc4, 0x6b, 0x1c, 
				0xab, 0x57, 0xa7, 0x93, 0x44, 0xb3, 0x4d, 0x39, 
				0x52, 0xf2, 0xdb, 0x11,
			};
		}
		namespace FrequencySweep {
			constexpr Packet_T configure {
				// 12 8-bit Read/Write Registers placeholder
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 

				0xFF,

				0xa8, 0xca, 0x9e, 0x91, 0x8e, 0xd3, 0x88, 
			};

			constexpr Packet_T initialize_with_start_freq {
				0x9f, 0x0b, 0x0a, 0x00, 0x55, 0xe6, 0x2b, 0xe7, 
				0x6f, 0x13, 0x94, 0x39, 0x85, 0x0e, 0xf4, 0x18, 
				0x25, 0x23, 0x69, 0x36,
			};

			constexpr Packet_T start {
				0x91, 0x12, 0x0c, 0xbc, 0x10, 0x8c, 0x75, 0xcb, 
				0x9f, 0x83, 0x91, 0x8c, 0x01, 0x7f, 0x43, 0x24, 
				0x31, 0x0a, 0x6a, 0x02,
			};
			constexpr Packet_T check_for_data_valid {
				0b0000'0000, // bool placeholder

				0xFF,

				0xc5, 0x00, 0x41, 0x6d, 0xf4, 0x3f, 0xc0, 0xcc, 
				0x4a, 0x9d, 0xe1, 0xdc, 0xf7, 0x16, 0x47, 0xfb,
				0x4a, 0xa6,
			};
			constexpr Packet_T read_data_valid_value {
				0x00, 0x00, // REAL_DATA (RAW)
				0x00, 0x00, // IMAG_DATA (RAW)
				0x00, 0x00, 0x00, 0x00, // REAL_DATA (float)
				0x00, 0x00, 0x00, 0x00, // IMAG_DATA (float)

				0xFF,

				0xb7, 0x02, 0x82, 0xf2, 0x19, 0xde, 0x45,
			};
			constexpr Packet_T repeat_freq {
				0x3f, 0xb7, 0x4d, 0xe0, 0xd8, 0x05, 0x71, 0x02, 
				0x87, 0xc9, 0xbb, 0xbb, 0x75, 0x35, 0xa5, 0x4c, 
				0x60, 0x04, 0x00, 0x9f,
			};
			constexpr Packet_T check_for_sweep_complete {
				0b0000'0000, // bool placeholder

				0xFF,

				0x5a, 0xec, 0xce, 0xf4, 0x1c, 0x80, 0xa3, 0xd2, 
				0x31, 0x39, 0xd5, 0xdf, 0x3f, 0xb1, 0x46, 0x8f, 
				0x7c, 0x2b, 	
			};
			constexpr Packet_T increment_frequency {
				0x4a, 0x7c, 0x2e, 0x99, 0xe7, 0x20, 0x07, 0xa5, 
				0x14, 0xe6, 0xdc, 0x49, 0x24, 0x94, 0x28, 0xfd, 
				0x48, 0x5c, 0xcf, 0x72,
			};
			constexpr Packet_T repeat_frequency_sweep {
				0xf3, 0x52, 0xb0, 0x32, 0x4a, 0x5c, 0x60, 0xb1, 
				0xf5, 0x52, 0x35, 0xbb, 0x95, 0x67, 0x1e, 0xb5, 
				0x3e, 0x67, 0xd4, 0xae,
			};
			constexpr Packet_T stop_frequency_sweep {
				0xa8, 0xed, 0x4b, 0x09, 0xe2, 0xc7, 0x7f, 0x13, 
				0xac, 0xb9, 0xae, 0xbc, 0x5c, 0x76, 0x94, 0x05, 
				0x45, 0x02, 0x59, 0x06,
			};
		}

		extern const std::array<const std::reference_wrapper<const Packet_T>, 15> all_packets;

		constexpr std::optional<size_t> find_footer_start_index(const Packet_T& raw_packet) {
			constexpr uint8_t footer_starter = 0xFF;
			const auto it { std::find(std::rbegin(raw_packet), std::rend(raw_packet), footer_starter) };

			if(it != std::rend(raw_packet)) {
				return std::distance(it, std::rend(raw_packet)) - 1;
			}

			return std::nullopt;
		}

		constexpr Packet_T get_raw_packet_footer(const Packet_T &in_raw_packet, const size_t footer_start_index) {
			Packet_T raw_packet = in_raw_packet;
			std::fill(raw_packet.begin(), raw_packet.begin() + footer_start_index, 0);
			return raw_packet;
		}

		constexpr std::optional<const Packet_T> get_magic_packet_pointer(const Packet_T &raw_packet) {
			const std::optional<size_t> footer_start_index { find_footer_start_index(raw_packet) };
			if(footer_start_index.has_value()) {
				const Packet_T packet_with_only_footer { get_raw_packet_footer(raw_packet, footer_start_index.value()) };
				for(size_t i = 0; i < all_packets.size(); i++) {
					if(packet_with_only_footer == all_packets[i].get()) {
						return all_packets[i];
					}
				}
			} else {
				for(size_t i = 0; i < all_packets.size(); i++) {
					if(raw_packet == all_packets[i].get()) {
						return all_packets[i];
					}
				}
			}
			return std::nullopt;
		}

		std::optional<std::vector<std::bitset<8>>> get_packet_data(const Packet_T &in_raw_packet);

		constexpr std::span<uint8_t> get_raw_packet_data(Packet_T &in_raw_packet, const size_t footer_start_index) {
			return std::span<uint8_t>(in_raw_packet.begin(), footer_start_index);
		}
	}
}

