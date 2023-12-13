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
			extern const Packet_T start;
			extern const Packet_T dump_all_registers;
			extern const Packet_T program_all_registers;
			extern const Packet_T control_HB_command;
			extern const Packet_T end;
		}
		namespace FrequencySweep {
			extern const Packet_T configure;
			extern const Packet_T initialize_with_start_freq;
			extern const Packet_T start;
			extern const Packet_T check_for_data_valid;
			extern const Packet_T read_data_valid_value;
			extern const Packet_T repeat_freq;
			extern const Packet_T check_for_sweep_complete;
			extern const Packet_T increment_frequency;
			extern const Packet_T repeat_frequency_sweep;
			extern const Packet_T stop_frequency_sweep;
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

