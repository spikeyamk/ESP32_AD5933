#include <iostream>
#include <array>
#include <cstdint>
#include <bitset>
#include <cassert>
#include <cmath>
#include <map>
#include <limits>

#include "ad5933_config.hpp"
#include "types.hpp"

const std::map<SettlingTimeCyclesMultiplierOrMask, const char*> SettlingTimeCyclesMultiplierOrMaskStringMap {
	{ SettlingTimeCyclesMultiplierOrMask::ONE_TIME, "ONE_TIME" },
	{ SettlingTimeCyclesMultiplierOrMask::TWO_TIMES, "TWO_TIMES" },
	{ SettlingTimeCyclesMultiplierOrMask::FOUR_TIMES, "FOUR_TIMES" },
};

const std::map<ControlHB::CommandOrMask, const char*> ControlHB_CommandOrMaskStringMap {
	{ ControlHB::CommandOrMask::NOP_0, "NOP_0" },
	{ ControlHB::CommandOrMask::INITIALIZE_WITH_START_FREQUENCY, "INITIALIZE_WITH_START_FREQUENCY" },
	{ ControlHB::CommandOrMask::START_FREQUENCY_SWEEP, "START_FREQUENCY_SWEEP" },
	{ ControlHB::CommandOrMask::INCREMENT_FREQUENCY, "INCREMENT_FREQUENCY" },
	{ ControlHB::CommandOrMask::REPEAT_FREQUENCY, "REPEAT_FREQUENCY" },
	{ ControlHB::CommandOrMask::NOP_1, "NOP_1" },
	{ ControlHB::CommandOrMask::MEASURE_TEMPERATURE, "MEASURE_TEMPERATURE" },
	{ ControlHB::CommandOrMask::POWER_DOWN_MODE, "POWER_DOWN_MODE" },
	{ ControlHB::CommandOrMask::STANDBY_MODE, "STANDBY_MODE" },
	{ ControlHB::CommandOrMask::NOP_2, "NOP_2" },
	{ ControlHB::CommandOrMask::NOP_3, "NOP_3" }
};

const std::map<ControlHB::OutputVoltageRangeOrMask, const char*> ControlHB_OutputVoltageRangeOrMaskStringMap {
	{ ControlHB::OutputVoltageRangeOrMask::TWO_VOLT_PPK, "TWO_VOLT_PPK" },
	{ ControlHB::OutputVoltageRangeOrMask::TWO_HUNDRED_MILI_VOLT_PPK, "TWO_HUNDRED_MILI_VOLT_PPK" },
	{ ControlHB::OutputVoltageRangeOrMask::FOUR_HUNDRED_MILI_VOLT_PPK, "FOUR_HUNDRED_MILI_VOLT_PPK" },
	{ ControlHB::OutputVoltageRangeOrMask::ONE_VOLT_PPK, "ONE_VOLT_PPK" },
};

const std::map<ControlHB::PGA_GainOrMask, const char*> ControlHB_PGA_GainOrMaskStringMap {
	{ ControlHB::PGA_GainOrMask::FIVE_TIMES, "FIVE_TIMES" },
	{ ControlHB::PGA_GainOrMask::ONE_TIME, "ONE_TIME" },
};

const std::map<ControlLB::SYSCLK_SRC_OrMask, const char*> ControlLB_SYSCLK_SRC_OrMaskStringMap {
	{ ControlLB::SYSCLK_SRC_OrMask::INTERNAL, "INTERNAL" },
	{ ControlLB::SYSCLK_SRC_OrMask::EXTERNAL, "EXTERNAL" }
};

const std::map<Status::STATUS_OrMask, const char*> STATUS_OrMaskStringMap {
    { Status::STATUS_OrMask::NO_STATUS, "NO_STATUS" },
    { Status::STATUS_OrMask::VALID_TEMP, "VALID_TEMP" },
    { Status::STATUS_OrMask::VALID_DATA, "VALID_DATA" },
    { Status::STATUS_OrMask::VALID_DATA_AND_VALID_TEMP, "VALID_DATA_AND_VALID_TEMP" },
    { Status::STATUS_OrMask::FREQ_SWEEP_COMPLETE, "FREQ_SWEEP_COMPLETE" },
    { Status::STATUS_OrMask::FREQ_SWEEP_COMPLETE_AND_VALID_TEMP, "FREQ_SWEEP_COMPLETE_AND_VALID_TEMP" },
    { Status::STATUS_OrMask::FREQ_SWEEP_COMPLETE_AND_VALID_DATA, "FREQ_SWEEP_COMPLETE_AND_VALID_DATA" },
    { Status::STATUS_OrMask::FREQ_SWEEP_COMPLETE_AND_VALID_DATA_AND_VALID_TEMP, "FREQ_SWEEP_COMPLETE_AND_VALID_DATA_AND_VALID_TEMP" },
};

std::ostream& operator<<(std::ostream& os, SYSCLK_FREQ value) {
    switch (value) {
        case SYSCLK_FREQ::EXTERNAL:
            os << "EXTERNAL";
            break;
        case SYSCLK_FREQ::INTERNAL:
            os << "INTERNAL";
            break;
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, SettlingTimeCyclesMultiplierOrMask value) {
    switch (value) {
        case SettlingTimeCyclesMultiplierOrMask::ONE_TIME:
            os << "ONE_TIME";
            break;
        case SettlingTimeCyclesMultiplierOrMask::TWO_TIMES:
            os << "TWO_TIMES";
            break;
        case SettlingTimeCyclesMultiplierOrMask::FOUR_TIMES:
            os << "FOUR_TIMES";
            break;
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, ControlHB::CommandOrMask value) {
    switch (value) {
        case ControlHB::CommandOrMask::NOP_0:
            os << "NOP_0";
            break;
        case ControlHB::CommandOrMask::INITIALIZE_WITH_START_FREQUENCY:
            os << "INITIALIZE_WITH_START_FREQUENCY";
            break;
        case ControlHB::CommandOrMask::START_FREQUENCY_SWEEP:
            os << "START_FREQUENCY_SWEEP";
            break;
        case ControlHB::CommandOrMask::INCREMENT_FREQUENCY:
            os << "INCREMENT_FREQUENCY";
            break;
        case ControlHB::CommandOrMask::REPEAT_FREQUENCY:
            os << "REPEAT_FREQUENCY";
            break;
        case ControlHB::CommandOrMask::NOP_1:
            os << "NOP_1";
            break;
        case ControlHB::CommandOrMask::MEASURE_TEMPERATURE:
            os << "MEASURE_TEMPERATURE";
            break;
        case ControlHB::CommandOrMask::POWER_DOWN_MODE:
            os << "POWER_DOWN_MODE";
            break;
        case ControlHB::CommandOrMask::STANDBY_MODE:
            os << "STANDBY_MODE";
            break;
        case ControlHB::CommandOrMask::NOP_2:
            os << "NOP_2";
            break;
        case ControlHB::CommandOrMask::NOP_3:
            os << "NOP_3";
            break;
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, ControlLB::SYSCLK_SRC_OrMask value) {
    switch (value) {
        case ControlLB::SYSCLK_SRC_OrMask::INTERNAL:
            os << "INTERNAL";
            break;
        case ControlLB::SYSCLK_SRC_OrMask::EXTERNAL:
            os << "EXTERNAL";
            break;
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, Status::STATUS_OrMask value) {
    switch (value) {
        case Status::STATUS_OrMask::NO_STATUS:
            os << "NO_STATUS";
            break;
        case Status::STATUS_OrMask::VALID_TEMP:
            os << "VALID_TEMP";
            break;
        case Status::STATUS_OrMask::VALID_DATA:
            os << "VALID_DATA";
            break;
        case Status::STATUS_OrMask::VALID_DATA_AND_VALID_TEMP:
            os << "VALID_DATA_AND_VALID_TEMP";
            break;
        case Status::STATUS_OrMask::FREQ_SWEEP_COMPLETE:
            os << "FREQ_SWEEP_COMPLETE";
            break;
        case Status::STATUS_OrMask::FREQ_SWEEP_COMPLETE_AND_VALID_TEMP:
            os << "FREQ_SWEEP_COMPLETE_AND_VALID_TEMP";
            break;
        case Status::STATUS_OrMask::FREQ_SWEEP_COMPLETE_AND_VALID_DATA:
            os << "FREQ_SWEEP_COMPLETE_AND_VALID_DATA";
            break;
        case Status::STATUS_OrMask::FREQ_SWEEP_COMPLETE_AND_VALID_DATA_AND_VALID_TEMP:
            os << "FREQ_SWEEP_COMPLETE_AND_VALID_DATA_AND_VALID_TEMP";
            break;
        default:
            os << "UNKNOWN";
            break;
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, ControlHB::OutputVoltageRangeOrMask value) {
    switch (value) {
        case ControlHB::OutputVoltageRangeOrMask::TWO_VOLT_PPK:
            os << "TWO_VOLT_PPK";
            break;
        case ControlHB::OutputVoltageRangeOrMask::ONE_VOLT_PPK:
            os << "ONE_VOLT_PPK";
            break;
        case ControlHB::OutputVoltageRangeOrMask::FOUR_HUNDRED_MILI_VOLT_PPK:
            os << "FOUR_HUNDRED_MILI_VOLT_PPK";
            break;
        case ControlHB::OutputVoltageRangeOrMask::TWO_HUNDRED_MILI_VOLT_PPK:
            os << "TWO_HUNDRED_MILI_VOLT_PPK";
            break;
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, ControlHB::PGA_GainOrMask value) {
    switch (value) {
        case ControlHB::PGA_GainOrMask::FIVE_TIMES:
            os << "FIVE_TIMES";
            break;
        case ControlHB::PGA_GainOrMask::ONE_TIME:
            os << "ONE_TIME";
            break;
    }
    return os;
}

AD5933_Config::AD5933_Config(
    const ControlHB::CommandOrMask command,
    const ControlHB::OutputVoltageRangeOrMask range,
    const ControlHB::PGA_GainOrMask pga_gain,
    const ControlLB::SYSCLK_SRC_OrMask sysclk_src,
    const uint32_t in_start_freq,
    const uint32_t in_inc_freq,
    const uint9_t in_num_of_inc,
    const uint9_t settling_time_cycles_number,
    const SettlingTimeCyclesMultiplierOrMask settling_time_cycles_multiplier,
	const int32_t calibration_impedance
) :
    control_HB { get_control_HB(command, range, pga_gain) },
    control_LB { get_control_LB(sysclk_src) },
	active_sysclk_freq { 
		sysclk_src == ControlLB::SYSCLK_SRC_OrMask::INTERNAL ? 
			static_cast<uint32_t>(SYSCLK_FREQ::INTERNAL) :
			static_cast<uint32_t>(SYSCLK_FREQ::EXTERNAL) 
	},
    start_freq { get_freq_register(in_start_freq) },
    inc_freq { get_freq_register(in_inc_freq) },
    num_of_inc { get_num_of_inc(in_num_of_inc) },
    settling_time_cycles { get_settling_time_cycles(settling_time_cycles_number, settling_time_cycles_multiplier) },
	calibration_impedance { calibration_impedance },
    ad5933_config_message { get_ad5933_config_message() },
    ad5933_config_message_raw { get_ad5933_config_message_raw() }
{}

AD5933_Config::AD5933_Config(const std::array<uint8_t, 12> &in_config_message_raw) :
	control_HB { std::bitset<8> { in_config_message_raw[0] } },
	control_LB { std::bitset<8> { in_config_message_raw[1] } },
	active_sysclk_freq { 
		static_cast<uint8_t>(ControlLB::SYSCLK_SRC_OrMask::INTERNAL) == static_cast<uint8_t>((control_LB.HB & std::bitset<8> { ControlLB::SYSCLK_SRC_AndMask }).to_ulong()) ? 
			static_cast<uint32_t>(SYSCLK_FREQ::INTERNAL) :
			static_cast<uint32_t>(SYSCLK_FREQ::EXTERNAL) 
	},
	start_freq { 
		std::bitset<8> { in_config_message_raw[2] },
		std::bitset<8> { in_config_message_raw[3] },
		std::bitset<8> { in_config_message_raw[4] }
	},
	inc_freq { 
		std::bitset<8> { in_config_message_raw[5] },
		std::bitset<8> { in_config_message_raw[6] },
		std::bitset<8> { in_config_message_raw[7] }
	},
	num_of_inc { 
		std::bitset<8> { in_config_message_raw[8] },
		std::bitset<8> { in_config_message_raw[9] }
	},
	settling_time_cycles { 
		std::bitset<8> { in_config_message_raw[10] },
		std::bitset<8> { in_config_message_raw[11] },
	},
	calibration_impedance { 0 },
	ad5933_config_message { get_ad5933_config_message() },
	ad5933_config_message_raw { get_ad5933_config_message_raw() }
{}

inline std::array<std::bitset<8>, 12> AD5933_Config::get_ad5933_config_message() const {
	return std::array<std::bitset<8>, 12> {
		control_HB.HB,
		control_LB.HB,
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

inline std::array<uint8_t, 12> AD5933_Config::get_ad5933_config_message_raw() const {
	std::array<uint8_t, 12> ret_val;
	for(size_t i = 0; i < 12; ++i) {
		ret_val[i] = static_cast<uint8_t>(ad5933_config_message[i].to_ulong());
	}
	return ret_val;
}

inline Register8_t AD5933_Config::get_control_HB(
	const Register8_t previous_state,
	const ControlHB::CommandOrMask command
) const {
	std::bitset<8> HB { previous_state.HB };
	HB |= std::bitset<8> { static_cast<uint8_t>(command) };
	return Register8_t { HB };
}

inline Register8_t AD5933_Config::get_control_HB(
	const Register8_t previous_state,
	const ControlHB::OutputVoltageRangeOrMask range
) const {
	std::bitset<8> HB { previous_state.HB };
	HB |= std::bitset<8> { static_cast<uint8_t>(range) };
	return Register8_t { HB };
}

inline Register8_t AD5933_Config::get_control_HB(
	const Register8_t previous_state,
	const ControlHB::PGA_GainOrMask pga_gain
) const {
	std::bitset<8> HB { previous_state.HB };
	HB |= std::bitset<8> { static_cast<uint8_t>(pga_gain) };
	return Register8_t { HB };

}

inline Register8_t AD5933_Config::get_control_HB(
	const Register8_t previous_state,
	const ControlHB::CommandOrMask command,
	const ControlHB::OutputVoltageRangeOrMask range
) const {
	std::bitset<8> HB { previous_state.HB };
	HB |= std::bitset<8> { static_cast<uint8_t>(command) };
	HB |= std::bitset<8> { static_cast<uint8_t>(range) };
	return Register8_t { HB };

}

inline Register8_t AD5933_Config::get_control_HB(
	const Register8_t previous_state,
	const ControlHB::CommandOrMask command,
	const ControlHB::PGA_GainOrMask pga_gain
) const {
	std::bitset<8> HB { previous_state.HB };
	HB |= std::bitset<8> { static_cast<uint8_t>(command) };
	HB |= std::bitset<8> { static_cast<uint8_t>(pga_gain) };
	return Register8_t { HB };

}

inline Register8_t AD5933_Config::get_control_HB(
	const Register8_t previous_state,
	const ControlHB::OutputVoltageRangeOrMask range,
	const ControlHB::PGA_GainOrMask pga_gain
) const {
	std::bitset<8> HB { previous_state.HB };
	HB |= std::bitset<8> { static_cast<uint8_t>(range) };
	HB |= std::bitset<8> { static_cast<uint8_t>(pga_gain) };
	return Register8_t { HB };

}

inline Register8_t AD5933_Config::get_control_HB(
	const ControlHB::CommandOrMask command,
	const ControlHB::OutputVoltageRangeOrMask range,
	const ControlHB::PGA_GainOrMask pga_gain
) const {
	std::bitset<8> HB { 0b0000'0000 };
	HB |= std::bitset<8> { static_cast<uint8_t>(command) };
	HB |= std::bitset<8> { static_cast<uint8_t>(range) };
	HB |= std::bitset<8> { static_cast<uint8_t>(pga_gain) };
	return Register8_t { HB };
}

inline Register8_t AD5933_Config::get_control_LB(const ControlLB::SYSCLK_SRC_OrMask sysclk_src) const {
	const std::bitset<8> HB { static_cast<uint8_t>(sysclk_src) };
	return Register8_t { HB };
} 

inline Register8_t AD5933_Config::get_control_LB(const ControlLB::SYSCLK_SRC_OrMask sysclk_src, bool reset) const {
	std::bitset<8> HB { static_cast<uint8_t>(sysclk_src) };
	if(reset == true) {
		HB |= ControlLB::RESET_SetMask;
	}
	return Register8_t { HB };
} 

inline Register8_t AD5933_Config::get_control_LB(const Register8_t previous_state, bool reset) const {
	std::bitset<8> HB { previous_state.HB };
	if(reset == true) {
		HB |= ControlLB::RESET_SetMask;
	}
	return Register8_t { HB };
} 

inline Register24_t AD5933_Config::get_freq_register(const uint32_t frequency) const {
	/*
	const uint24_t binary_coded_frequency = static_cast<uint24_t>(
		static_cast<double>(frequency * 4) *
		static_cast<double>((static_cast<double>(pow_2_27) / static_cast<double>(active_sysclk_freq))
		)
	);
	return Register24_t { 
		binary_coded_frequency.get_HB(),
		binary_coded_frequency.get_MB(),
		binary_coded_frequency.get_LB(),
	};
	*/

	const uint32_t binary_coded_frequency = (uint32_t)((double)frequency * 4 /
		static_cast<double>(active_sysclk_freq) * pow_2_27);
	const uint32_t twenty_four_bit_max_number = 0xFFFFFF;

	if(binary_coded_frequency > twenty_four_bit_max_number) {
		std::cout << "AD5933: Overflow: Failed to convert frequency to binary coded frequency message from frequency: " << frequency << std::endl;
		//return std::nullopt;
	} //else {
	const std::array<uint8_t, 3> haha {
			(uint8_t) ((binary_coded_frequency  >> (2 * 8)) & 0xFF),
			(uint8_t) ((binary_coded_frequency  >> (1 * 8)) & 0xFF),
			(uint8_t) ((binary_coded_frequency  >> (0 * 8)) & 0xFF)
	};
	return Register24_t {
		std::bitset<8> { haha[0] },
		std::bitset<8> { haha[1] },
		std::bitset<8> { haha[2] }
	};
}

inline Register16_t AD5933_Config::get_num_of_inc(const uint9_t in_num_of_inc) const {
	std::bitset<8> HB { 0b0000'0000 };
	auto tmp = in_num_of_inc.get_bit_8_MSB()[0];
	HB[0] = tmp;

	const std::bitset<8> LB { in_num_of_inc.get_bit_7_to_0_LB() };

	return Register16_t { HB, LB };
}

inline Register16_t AD5933_Config::get_settling_time_cycles(const Register16_t previous_state, const uint9_t number) const {
	std::bitset<8> HB { previous_state.HB };
	HB[0] = number.get_bit_8_MSB()[0];

	const std::bitset<8> LB { number.get_bit_7_to_0_LB() };

	return Register16_t { HB , LB };
}

inline Register16_t AD5933_Config::get_settling_time_cycles(const Register16_t previous_state, const SettlingTimeCyclesMultiplierOrMask multiplier) const {
	std::bitset<8> HB { previous_state.HB };
	HB |= std::bitset<8>{ static_cast<uint8_t>(multiplier) };

	const std::bitset<8> LB { previous_state.LB };

	return Register16_t { HB , LB };
}

inline Register16_t AD5933_Config::get_settling_time_cycles(const uint9_t number, const SettlingTimeCyclesMultiplierOrMask multiplier) const {
	std::bitset<8> HB { 0b0000'0000 };
	HB[0] = number.get_bit_8_MSB()[0];
	HB |= std::bitset<8>{ static_cast<uint8_t>(multiplier) };

	const std::bitset<8> LB { number.get_bit_7_to_0_LB() };

	return Register16_t { HB , LB };
}

inline ControlHB::CommandOrMask AD5933_Config::get_command() const {
	const std::bitset<8> command_state { control_HB.HB & ControlHB::CommandAndMask };
	return ControlHB::CommandOrMask(static_cast<uint8_t>(command_state.to_ulong()));
}

inline ControlHB::OutputVoltageRangeOrMask AD5933_Config::get_voltage_range() const {
	const std::bitset<8> voltage_range_state { control_HB.HB & ControlHB::OutputVoltageRangeAndMask };
	return ControlHB::OutputVoltageRangeOrMask(static_cast<uint8_t>(voltage_range_state.to_ulong()));
}

inline ControlHB::PGA_GainOrMask AD5933_Config::get_pga_gain() const {
	const std::bitset<8> pga_gain_state { control_HB.HB & ControlHB::PGA_GainAndMask };
	return ControlHB::PGA_GainOrMask(static_cast<uint8_t>(pga_gain_state.to_ulong()));
}

inline ControlLB::SYSCLK_SRC_OrMask AD5933_Config::get_sysclk_src() const {
	const std::bitset<8> sysclk_src_state { control_LB.HB & ControlLB::SYSCLK_SRC_AndMask };
	return ControlLB::SYSCLK_SRC_OrMask(static_cast<uint8_t>(sysclk_src_state.to_ulong()));
}

bool AD5933_Config::get_reset() const {
	const std::bitset<8> reset_state { control_LB.HB & ControlLB::RESET_ExtractMask };
	if(reset_state == ControlLB::RESET_SetMask) {
		return true;
	} else {
		return false;
	}
}

inline SettlingTimeCyclesMultiplierOrMask AD5933_Config::get_settling_time_cycles_multiplier() const {
	const std::bitset<8> settling_time_cycles_multiplier_state { settling_time_cycles.HB & SettlingTimeCyclesMultiplierAndMask };
	return SettlingTimeCyclesMultiplierOrMask(static_cast<uint8_t>(settling_time_cycles_multiplier_state.to_ulong()));
}

inline uint9_t AD5933_Config::get_uint9_t(const Register16_t state) const {
   	const std::bitset<16> HB_LB_combined { state.get_HB_LB_combined_bitset() };
	const uint16_t HB_LB_comined_raw = static_cast<uint16_t>(HB_LB_combined.to_ulong());
	return static_cast<uint9_t>(HB_LB_comined_raw);
}

inline uint9_t AD5933_Config::get_settling_time_cycles_number() const {
	return get_uint9_t(settling_time_cycles);
}

inline uint9_t AD5933_Config::get_num_of_inc() const {
	return get_uint9_t(num_of_inc);
}

inline double AD5933_Config::get_freq(const Register24_t state) const {
	const std::bitset<24> HB_MB_LB_combined { state.get_HB_MB_LB_combined_bitset() };
	/*
	const uint24_t HB_MB_LB_combined_raw = static_cast<uint24_t>(HB_MB_LB_combined.to_ulong());
	return static_cast<double> (
		static_cast<double>((static_cast<__int128_t>(HB_MB_LB_combined_raw.value) * static_cast<__int128_t>(active_sysclk_freq))) /
		static_cast<double>((static_cast<__int128_t>(static_cast<__int128_t>(4) * static_cast<__int128_t>(pow_2_27))))
	);
	*/

	const uint32_t binary_coded_frequency = HB_MB_LB_combined.to_ulong();
	const uint32_t twenty_four_bit_max_number = 0xFFFFFF;
	if(binary_coded_frequency > twenty_four_bit_max_number) {
		std::cout << "AD5933: Overflow: Failed to convert binary coded frequency message to frequency from: " << HB_MB_LB_combined << std::endl;
	}
	double frequency = static_cast<double>(
		(
			(static_cast<double>(binary_coded_frequency) / 4.00f) *
			static_cast<double>(active_sysclk_freq)
		) / static_cast<double>(pow_2_27)
	);
	return frequency;
}

inline double AD5933_Config::get_start_freq() const {
	return std::round(get_freq(start_freq));
}

inline double AD5933_Config::get_inc_freq() const {
	return std::round(get_freq(inc_freq));
}

void AD5933_Config::print() const {
	std::cout << "Command: " << get_command() << std::endl;
	std::cout << "Range: " << get_voltage_range() << std::endl;
	std::cout << "PGA Gain: " << get_pga_gain() << std::endl;
	std::cout << "System clock: " << get_sysclk_src() << std::endl;
	std::cout << "System clock frequency: " << active_sysclk_freq << std::endl;
	std::cout << "Start frequency: " << get_start_freq() << std::endl;
	std::cout << "Increment frequency: " << get_inc_freq() << std::endl;
	std::cout << "Number of increments: " << get_num_of_inc() << std::endl;
	std::cout << "Settling cycles number: " << get_settling_time_cycles_number() << std::endl;
	std::cout << "Settling cycles multiplier: " << get_settling_time_cycles_multiplier() << std::endl;
}

void AD5933_Config::set_command(ControlHB::CommandOrMask command) {
	control_HB = get_control_HB(control_HB, command);
}
void AD5933_Config::set_voltage_range(ControlHB::OutputVoltageRangeOrMask range) {
	control_HB = get_control_HB(control_HB, range);
}

void AD5933_Config::set_pga_gain(ControlHB::PGA_GainOrMask pga_gain) {
	control_HB = get_control_HB(control_HB, pga_gain);
}

void AD5933_Config::set_sysclk_src(ControlLB::SYSCLK_SRC_OrMask sysclk_src) {
	control_LB = get_control_LB(sysclk_src);
	switch(sysclk_src) {
		case ControlLB::SYSCLK_SRC_OrMask::EXTERNAL:
			active_sysclk_freq = static_cast<uint32_t>(SYSCLK_FREQ::EXTERNAL);
			break;
		case ControlLB::SYSCLK_SRC_OrMask::INTERNAL:
			active_sysclk_freq = static_cast<uint32_t>(SYSCLK_FREQ::INTERNAL);
			break;
	}
}

void AD5933_Config::set_settling_time_cycles_multiplier(SettlingTimeCyclesMultiplierOrMask multiplier) {
	settling_time_cycles = get_settling_time_cycles(settling_time_cycles, multiplier);
}

void AD5933_Config::set_settling_time_cycles_number(uint9_t number) {
	settling_time_cycles = get_settling_time_cycles(settling_time_cycles, number);
}

void AD5933_Config::set_num_of_inc(uint9_t num) {
	num_of_inc = get_num_of_inc(num);
}

void AD5933_Config::set_start_freq(uint32_t freq) {
	start_freq = get_freq_register(freq);
}

void AD5933_Config::set_inc_freq(uint32_t freq) {
	inc_freq = get_freq_register(freq);
}

AD5933_Config AD5933_Config::get_default() {
	const ControlHB::CommandOrMask command = ControlHB::CommandOrMask::POWER_DOWN_MODE;
	const ControlHB::OutputVoltageRangeOrMask range = ControlHB::OutputVoltageRangeOrMask::TWO_VOLT_PPK;
	const ControlHB::PGA_GainOrMask pga_gain = ControlHB::PGA_GainOrMask::ONE_TIME;
	const ControlLB::SYSCLK_SRC_OrMask sysclk_src = ControlLB::SYSCLK_SRC_OrMask::INTERNAL;
	const uint32_t start_freq = 30'000;
	const uint32_t inc_freq = 10;
	const uint9_t num_of_inc = 2;
	const uint9_t settling_cycles_number = 15;
	const SettlingTimeCyclesMultiplierOrMask settling_cycles_multiplier = SettlingTimeCyclesMultiplierOrMask::ONE_TIME;
	const uint32_t calibration_impedance = 220'000;
	AD5933_Config default_config (
		command,
		range,
		pga_gain,
		sysclk_src,
		start_freq,
		inc_freq,
		num_of_inc,
		settling_cycles_number,
		settling_cycles_multiplier,
		calibration_impedance
	);
	return default_config;
}

AD5933_Data::AD5933_Data(const std::array<uint8_t, 7> &in_data_message_raw) :
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
	}
{}

Status::STATUS_OrMask AD5933_Data::get_status() {
	const std::bitset<8> status_state { status.HB & Status::STATUS_AndMask };
	return Status::STATUS_OrMask(static_cast<uint8_t>(status_state.to_ulong()));
}

float AD5933_Data::get_temperature() {
	uint16_t temp_data_num = static_cast<uint16_t>(temp_data.get_HB_LB_combined_bitset().to_ulong());
	if(temp_data_num < 8192) {
		return (static_cast<float>(temp_data_num) / 32.0f);
	} else {
		return (static_cast<float>((temp_data_num - 16384)) / 32.0f);
	}
}

int16_t AD5933_Data::get_real_part() {
	return static_cast<int16_t>(real_data.get_HB_LB_combined_bitset().to_ulong());
} 

int16_t AD5933_Data::get_imag_part() {
	return static_cast<int16_t>(imag_data.get_HB_LB_combined_bitset().to_ulong());
}

float AD5933_Data::get_raw_magnitude() {
	float real_part = static_cast<float>(get_real_part());
	float imag_part = static_cast<float>(get_imag_part());
	return std::sqrtf( (real_part * real_part) + (imag_part * imag_part) );
}

float AD5933_Data::get_raw_phase() {
	// UNIMPLEMENTED
	float real_part = static_cast<float>(get_real_part());
	float imag_part = static_cast<float>(get_imag_part());
	return std::atan2f(imag_part, real_part);
}

AD5933_CalibrationData::AD5933_CalibrationData(
	const uint8_t real_data_HB,
	const uint8_t real_data_LB,
	const uint8_t imag_data_HB,
	const uint8_t imag_data_LB
) :
	AD5933_Data {
		std::array<uint8_t, 7> {
			0x00u, 0x00u, 0x00u,
			real_data_HB,
			real_data_LB,
			imag_data_HB,
			imag_data_LB
		}
	}
{}


float AD5933_CalibrationData::get_gain_factor(const int32_t calibration_impedance) {
	return (1.00f / static_cast<float>(calibration_impedance)) / get_raw_magnitude();
}

float AD5933_CalibrationData::get_system_phase() {
	return get_raw_phase();
}

AD5933_MeasurementData::AD5933_MeasurementData(
	const uint8_t real_data_HB,
	const uint8_t real_data_LB,
	const uint8_t imag_data_HB,
	const uint8_t imag_data_LB
) :
	AD5933_Data {
		std::array<uint8_t, 7> {
			0x00u, 0x00u, 0x00u,
			real_data_HB,
			real_data_LB,
			imag_data_HB,
			imag_data_LB
		}
	}
{}

float AD5933_MeasurementData::get_corrected_magnitude(const float gain_factor) {
	return (1.00f / (gain_factor * get_raw_magnitude()));
}

float AD5933_MeasurementData::get_corrected_phase(const float system_phase) {
	return get_raw_phase() - system_phase;
}

float AD5933_MeasurementData::get_corrected_resistance(const float gain_factor, const float system_phase) {
	const float magnitude = get_corrected_magnitude(gain_factor);
	const float phase = get_corrected_phase(system_phase);
	return (magnitude * std::cosf(phase));
}

float AD5933_MeasurementData::get_corrected_reactance(const float gain_factor, const float system_phase) {
	const float magnitude = get_corrected_magnitude(gain_factor);
	const float phase = get_corrected_phase(system_phase);
	return (magnitude * std::sinf(phase));
}
