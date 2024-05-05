#pragma once

#include <array>
#include <bitset>
#include <cstdint>

#include "ad5933/register_types.hpp"
#include "ad5933/temperature/temperature.hpp"
#include "ad5933/data/data.hpp"
#include "ad5933/masks/maps.hpp"

namespace AD5933 {
	template<typename T_Floating>
	class DebugData {
	private:
		AD5933::Register8_t status;
		AD5933::Register16_t temp_data;
		AD5933::Register16_t real_data;
		AD5933::Register16_t imag_data;
		AD5933::Temperature<T_Floating> temperature { std::array<uint8_t, 2> { 0x00, 0x00 }};
		AD5933::Data data { 0x00, 0x00 };
	public:
		DebugData() = default;

		DebugData(const std::array<uint8_t, 7> &in_data_message_raw) :
			status { std::bitset<8> { in_data_message_raw[0] } },
			temp_data { 
				std::bitset<8> { in_data_message_raw[1]},
				std::bitset<8> { in_data_message_raw[2]},
			},
			real_data { 
				std::bitset<8> { in_data_message_raw[3]},
				std::bitset<8> { in_data_message_raw[4]},
			},
			imag_data { 
				std::bitset<8> { in_data_message_raw[5]},
				std::bitset<8> { in_data_message_raw[6]},
			},
			temperature {
				std::array<uint8_t, 2> { in_data_message_raw[1], in_data_message_raw[2] }
			},
			data {
				std::array<uint8_t, 4> { 
					in_data_message_raw[3],
					in_data_message_raw[4],
					in_data_message_raw[5],
					in_data_message_raw[6],
				}
			}
		{}

		AD5933::Masks::Or::Status get_status() const {
			const std::bitset<8> status_state { status.HB & AD5933::Masks::And::Status };
			return AD5933::Masks::Or::Status(static_cast<uint8_t>(status_state.to_ulong()));
		}

		int16_t get_real_part() const {
			return data.get_real_data();
		} 

		int16_t get_imag_part() const {
			return data.get_imag_data();
		}

		T_Floating get_temperature() const {
			return temperature.get_value();
		}

		T_Floating get_raw_magnitude() const {
			return data.get_raw_magnitude<T_Floating>();
		}

		T_Floating get_raw_phase() const {
			return data.get_raw_phase<T_Floating>();
		}
	};
}