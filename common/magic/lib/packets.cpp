#include <array>
#include <optional>
#include <cstdint>
#include <algorithm>
#include <bitset>
#include <span>

#include "magic/packets.hpp"

namespace Magic {
	namespace Packets {
		namespace Debug {

		}

		namespace FrequencySweep {

		}

		constexpr std::array<const std::reference_wrapper<const Packet_T>, 15> all_packets {
			Debug::start,
			Debug::dump_all_registers,
			Debug::program_all_registers,
			Debug::control_HB_command,
			Debug::end,

			FrequencySweep::configure,
			FrequencySweep::initialize_with_start_freq,
			FrequencySweep::start,
			FrequencySweep::check_for_data_valid,
			FrequencySweep::read_data_valid_value,
			FrequencySweep::repeat_freq,
			FrequencySweep::check_for_sweep_complete,
			FrequencySweep::increment_frequency,
			FrequencySweep::repeat_frequency_sweep,
			FrequencySweep::stop_frequency_sweep
		};
	}
}

namespace Magic {
	namespace Packets {
		std::optional<std::vector<std::bitset<8>>> get_packet_data(const Packet_T &in_raw_packet) {
			const std::optional<size_t> footer_start_index = find_footer_start_index(in_raw_packet);
			if(footer_start_index.has_value() == false) {
				return std::nullopt;
			}
			const auto raw_packet_data { get_raw_packet_data(const_cast<Packet_T&>(in_raw_packet), footer_start_index.value()) };
			std::vector<std::bitset<8>> ret;
			for(const auto i: raw_packet_data) {
				ret.push_back(std::bitset<8> { i });
			}
			return ret;
		}
	}
}
