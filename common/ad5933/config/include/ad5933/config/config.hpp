#pragma once

#include <cstdint>
#include <array>
#include <cmath>
#include <vector>
#include <algorithm>
#include <type_traits>
#include <concepts>

#include "ad5933/register_types.hpp"
#include "ad5933/uint_types.hpp"
#include "ad5933/masks/masks.hpp"

namespace AD5933 {
    enum class SysClkFreq : uint32_t {
        External = 16'000'000ul,
        Internal = 16'776'000ul
    };

	class POW_2_29_DividedBySysClkFreq {
	private:
		static constexpr float POW_2_29 = static_cast<float>(1 << 29);
		static constexpr float External = (POW_2_29 / static_cast<float>(SysClkFreq::External));
		static constexpr float Internal = (POW_2_29 / static_cast<float>(SysClkFreq::Internal));
	public:
		static inline constexpr float get(const SysClkFreq freq_hz) {
			switch(freq_hz) {
				case SysClkFreq::External:
					return External;
				case SysClkFreq::Internal:
					return Internal;
				default:
					return 0;
			}
		}
	};

	class Config {
	public:
		Register16_t ctrl;
		Register24_t start_freq;
		Register24_t inc_freq;
		Register16_t num_of_inc;
		Register16_t settling_time_cycles;

		inline constexpr Config(
			const Masks::Or::Ctrl::HB::Command command = Masks::Or::Ctrl::HB::Command::PowerDownMode,
			const Masks::Or::Ctrl::HB::VoltageRange range = Masks::Or::Ctrl::HB::VoltageRange::Two_Vppk,
			const Masks::Or::Ctrl::HB::PGA_Gain pga_gain = Masks::Or::Ctrl::HB::PGA_Gain::OneTime,
			const Masks::Or::Ctrl::LB::SysClkSrc sysclk_src = Masks::Or::Ctrl::LB::SysClkSrc::Internal,

			const uint_startfreq_t start_freq = uint_startfreq_t { 30'000 },
			const uint_incfreq_t inc_freq = uint_incfreq_t { 10 },
			const uint9_t num_of_inc = uint9_t { 2 },

			const uint9_t settling_time_cycles_number = uint9_t { 15 },
			const Masks::Or::SettlingTimeCyclesHB::Multiplier settling_time_cycles_multiplier = Masks::Or::SettlingTimeCyclesHB::Multiplier::OneTime
		) :
			ctrl{get_ctrl(command, range, pga_gain, sysclk_src)},
			start_freq{ get_freq_register(start_freq) },
			inc_freq{ get_freq_register(inc_freq) },
			num_of_inc{ get_num_of_inc(num_of_inc) },
			settling_time_cycles{ get_settling_time_cycles(settling_time_cycles_number, settling_time_cycles_multiplier) }
		{}

		inline constexpr Config(
			const std::array<uint8_t, 12> raw_bytes
		) :
			ctrl{
				std::bitset<8> { raw_bytes[0] },
				std::bitset<8> { raw_bytes[1] } 
			},
			start_freq{ 
				std::bitset<8> { raw_bytes[2] },
				std::bitset<8> { raw_bytes[3] },
				std::bitset<8> { raw_bytes[4] } 
			},
			inc_freq{ 
				std::bitset<8> { raw_bytes[5] },
				std::bitset<8> { raw_bytes[6] },
				std::bitset<8> { raw_bytes[7] } 
			},
			num_of_inc{ 
				std::bitset<8> { raw_bytes[8] },
				std::bitset<8> { raw_bytes[9] },
			},
			settling_time_cycles{ 
				std::bitset<8> { raw_bytes[10] },
				std::bitset<8> { raw_bytes[11] },
			}
		{}
	public:
		constexpr std::array<uint8_t, 12> to_raw_array() const {
			return std::array<uint8_t, 12> {
				static_cast<uint8_t>(ctrl.HB.to_ulong()),
				static_cast<uint8_t>(ctrl.LB.to_ulong()),
				static_cast<uint8_t>(start_freq.HB.to_ulong()),
				static_cast<uint8_t>(start_freq.MB.to_ulong()),
				static_cast<uint8_t>(start_freq.LB.to_ulong()),
				static_cast<uint8_t>(inc_freq.HB.to_ulong()),
				static_cast<uint8_t>(inc_freq.MB.to_ulong()),
				static_cast<uint8_t>(inc_freq.LB.to_ulong()),
				static_cast<uint8_t>(num_of_inc.HB.to_ulong()),
				static_cast<uint8_t>(num_of_inc.LB.to_ulong()),
				static_cast<uint8_t>(settling_time_cycles.HB.to_ulong()),
				static_cast<uint8_t>(settling_time_cycles.LB.to_ulong())
			};
		}

		constexpr std::array<std::bitset<8>, 12> to_bitset_array() const {
			return std::array<std::bitset<8>, 12> {
				ctrl.HB,
				ctrl.LB,
				start_freq.HB,
				start_freq.MB,
				start_freq.LB,
				inc_freq.HB,
				inc_freq.MB,
				inc_freq.LB,
				num_of_inc.HB,
				num_of_inc.LB,
				settling_time_cycles.HB,
				settling_time_cycles.LB
			};
		}
	public:
		constexpr Masks::Or::Ctrl::HB::Command get_command() const {
			return Masks::Or::Ctrl::HB::Command((ctrl.HB & Masks::And::Ctrl::HB::Command).to_ulong());
		}

		constexpr Masks::Or::Ctrl::HB::VoltageRange get_voltage_range() const {
			return Masks::Or::Ctrl::HB::VoltageRange ((ctrl.HB & Masks::And::Ctrl::HB::VoltageRange).to_ulong());
		}	

		constexpr Masks::Or::Ctrl::HB::PGA_Gain get_PGA_gain() const {
			return Masks::Or::Ctrl::HB::PGA_Gain((ctrl.HB & Masks::And::Ctrl::HB::PGA_Gain).to_ulong());
		}

		constexpr Masks::Or::Ctrl::LB::SysClkSrc get_sysclk_src() const {
			return Masks::Or::Ctrl::LB::SysClkSrc ((ctrl.LB & Masks::And::Ctrl::LB::SysClkSrc).to_ulong());
		}

		template<typename T_Freq>
		constexpr T_Freq get_freq(const Register24_t &reg) const {
			const uint32_t HB = static_cast<uint8_t>(reg.HB.to_ulong());
			const uint32_t MB = static_cast<uint8_t>(reg.MB.to_ulong());
			const uint32_t LB = static_cast<uint8_t>(reg.LB.to_ulong());
			const uint32_t binary_coded_frequency {
				LB
				| (MB << 8)
				| (HB << 16)
			};
			return T_Freq {
				static_cast<uint32_t>(std::round(
					static_cast<float>(binary_coded_frequency)
					/ POW_2_29_DividedBySysClkFreq::get(get_active_sysclk_freq()))
				)
			};
		}

		constexpr uint_startfreq_t get_start_freq() const {
			return get_freq<uint_startfreq_t>(start_freq);
		}

		constexpr uint_incfreq_t get_inc_freq() const {
			return get_freq<uint_incfreq_t>(inc_freq);
		}
		
		constexpr uint9_t get_num_of_inc() const {
			const uint9_t HB {
				static_cast<uint16_t>(num_of_inc.HB.to_ulong())
			};
			uint9_t LB {
				static_cast<uint16_t>(num_of_inc.LB.to_ulong())
			};
			LB |= (HB << 8);
			return LB;
		}

		constexpr uint9_t get_settling_time_cycles_number() const {
			const uint9_t HB {
				static_cast<uint16_t>((settling_time_cycles.HB & Masks::And::SettlingTimeCyclesHB::Number).to_ulong())
			};
			uint9_t LB {
				static_cast<uint16_t>(settling_time_cycles.LB.to_ulong())
			};
			LB |= (HB << 8);
			return LB;
		}

		constexpr Masks::Or::SettlingTimeCyclesHB::Multiplier get_settling_time_cycles_multiplier() const {
			return Masks::Or::SettlingTimeCyclesHB::Multiplier((settling_time_cycles.HB & Masks::And::SettlingTimeCyclesHB::Multiplier).to_ulong());
		}
	public:
		constexpr void set_command(const Masks::Or::Ctrl::HB::Command command) {
			const std::bitset<8> set_mask { static_cast<uint8_t>(command) };
			ctrl.HB = ((ctrl.HB & ~Masks::And::Ctrl::HB::Command) | set_mask);
		}
		
		constexpr void set_voltage_range(const Masks::Or::Ctrl::HB::VoltageRange range) {
			const std::bitset<8> set_mask { static_cast<uint8_t>(range) };
			ctrl.HB = ((ctrl.HB & ~Masks::And::Ctrl::HB::VoltageRange) | set_mask);
		}

		constexpr void set_PGA_gain(const Masks::Or::Ctrl::HB::PGA_Gain pga_gain) {
			const std::bitset<8> set_mask { static_cast<uint8_t>(pga_gain) };
			ctrl.HB = ((ctrl.HB & ~Masks::And::Ctrl::HB::PGA_Gain) | set_mask);
		}

		constexpr void set_sysclk_src(const Masks::Or::Ctrl::LB::SysClkSrc sysclk_src) {
			const uint_startfreq_t start_freq { get_start_freq() };
			const uint_incfreq_t inc_freq { get_inc_freq() };
			const std::bitset<8> set_mask { static_cast<uint8_t>(sysclk_src) };
			ctrl.LB = ((ctrl.LB & ~Masks::And::Ctrl::LB::SysClkSrc) | set_mask);
			set_start_freq(start_freq);
			set_inc_freq(inc_freq);
		}

		constexpr void set_start_freq(const uint_startfreq_t &in_start_freq) {
			start_freq = get_freq_register<uint_startfreq_t>(in_start_freq);
		}

		constexpr void set_inc_freq(const uint_incfreq_t &in_inc_freq) {
			inc_freq = get_freq_register<uint_incfreq_t>(in_inc_freq);
		}

		constexpr void set_num_of_inc(const uint9_t &in_num_of_inc) {
			num_of_inc = get_num_of_inc(in_num_of_inc);
		}

		constexpr void set_settling_time_cycles_number(const uint9_t &in_settling_time_cycles_number) {
			settling_time_cycles = get_settling_time_cycles(in_settling_time_cycles_number, get_settling_time_cycles_multiplier());
		}

		constexpr void set_settling_time_cycles_multiplier(const Masks::Or::SettlingTimeCyclesHB::Multiplier multiplier) {
			const std::bitset<8> set_mask { static_cast<uint8_t>(multiplier) };
			settling_time_cycles.HB = ((settling_time_cycles.HB & ~Masks::And::SettlingTimeCyclesHB::Multiplier) | set_mask);
		}

		inline constexpr SysClkFreq get_active_sysclk_freq() const {
			switch(Masks::Or::Ctrl::LB::SysClkSrc((ctrl.LB & Masks::And::Ctrl::LB::SysClkSrc).to_ulong())) {
				case Masks::Or::Ctrl::LB::SysClkSrc::External:
					return SysClkFreq::External;
				default:
					return SysClkFreq::Internal;
			}
		}
	private:
		inline constexpr Register16_t get_ctrl(
			const Masks::Or::Ctrl::HB::Command command,
			const Masks::Or::Ctrl::HB::VoltageRange range,
			const Masks::Or::Ctrl::HB::PGA_Gain pga_gain,
			const Masks::Or::Ctrl::LB::SysClkSrc sysclk_src
		) const {
			return Register16_t {
				std::bitset<8>(static_cast<uint8_t>(command)
				| static_cast<uint8_t>(range)
				| static_cast<uint8_t>(pga_gain)),
				std::bitset<8>(static_cast<uint8_t>(sysclk_src))
			};
		}

		template<typename T_Freq>
		inline constexpr Register24_t get_freq_register(
			const T_Freq freq_hz
		) const {
			const uint32_t binary_coded_frequency = static_cast<uint32_t>(freq_hz.to_float() * POW_2_29_DividedBySysClkFreq::get(get_active_sysclk_freq()));
			const uint8_t HB = static_cast<uint8_t>((binary_coded_frequency & 0xFF'00'00) >> 16);
			const uint8_t MB = static_cast<uint8_t>((binary_coded_frequency & 0x00'FF'00) >> 8);
			const uint8_t LB = static_cast<uint8_t>(binary_coded_frequency & 0x00'00'FF);
			return {
				std::bitset<8> { HB },
				std::bitset<8> { MB },
				std::bitset<8> { LB }
			};
		}

		inline constexpr Register16_t get_num_of_inc(
			const uint9_t num_of_inc
		) const {
			return Register16_t {
				num_of_inc.get_bit_8_MSB(),
				num_of_inc.get_bit_7_to_0_LB()
			};
		}

		inline constexpr Register16_t get_settling_time_cycles(
			const uint9_t num_of_cycles,
			const Masks::Or::SettlingTimeCyclesHB::Multiplier multiplier
		) const {
			const std::bitset<8> mask { static_cast<uint8_t>(multiplier) };
			return Register16_t {
				(num_of_cycles.get_bit_8_MSB() | mask),
				num_of_cycles.get_bit_7_to_0_LB()
			};
		}
	public:
		void print() const;	
	public:
		constexpr size_t get_freq_container_size() const {
			return get_num_of_inc().unwrap() + 1;
		}

		template<typename T>
		constexpr std::vector<T> get_freq_vector() const {
			std::vector<T> ret;
			ret.resize(get_freq_container_size());
            std::generate(
				ret.begin(),
				ret.end(),
				[
					start_freq = get_start_freq(),
					inc_freq = get_inc_freq(),
					n = static_cast<T>(0.0f)
				] () mutable {
					return static_cast<T>(start_freq.unwrap() + ((n++) * inc_freq.unwrap()));
				}
			);
			return ret;
		}

		constexpr uint_freq_t<1001> get_freq_end() const {
            return uint_freq_t<1001> { get_start_freq().unwrap() + (get_num_of_inc().unwrap() * get_inc_freq().unwrap()) };
		}
	};
}