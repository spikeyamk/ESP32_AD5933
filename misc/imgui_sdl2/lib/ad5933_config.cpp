#include <iostream>
#include <array>
#include <cstdint>
#include <bitset>
#include <cassert>
#include <cmath>

#include "ad5933_config.hpp"
#include "types.hpp"

const std::map<SettlingTimeCyclesMultiplierOrMask, const char*> SettlingTimeCyclesMultiplierOrMaskStringMap {
	{ SettlingTimeCyclesMultiplierOrMask::ONE_TIME, "ONE_TIME" },
	{ SettlingTimeCyclesMultiplierOrMask::TWO_TIMES, "TWO_TIMES" },
	{ SettlingTimeCyclesMultiplierOrMask::FOUR_TIMES, "FOUR_TIMES" }
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
	{ ControlHB::OutputVoltageRangeOrMask::ONE_VOLT_PPK, "ONE_VOLT_PPK" },
	{ ControlHB::OutputVoltageRangeOrMask::FOUR_HUNDRED_MILI_VOLT_PPK, "FOUR_HUNDRED_MILI_VOLT_PPK" },
	{ ControlHB::OutputVoltageRangeOrMask::TWO_HUNDRED_MILI_VOLT_PPK, "TWO_HUNDRED_MILI_VOLT_PPK" }
};

const std::map<ControlHB::PGA_GainOrMask, const char*> ControlHB_PGA_GainOrMaskStringMap {
	{ ControlHB::PGA_GainOrMask::ONE_TIME, "ONE_TIME" },
	{ ControlHB::PGA_GainOrMask::FIVE_TIMES, "FIVE_TIMES" }
};

const std::map<ControlLB::SYSCLK_SRC_OrMask, const char*> ControlLB_SYSCLK_SRC_OrMaskStringMap {
	{ ControlLB::SYSCLK_SRC_OrMask::INTERNAL, "INTERNAL" },
	{ ControlLB::SYSCLK_SRC_OrMask::EXTERNAL, "EXTERNAL" }
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
    const SettlingTimeCyclesMultiplierOrMask settling_time_cycles_multiplier
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
	const uint32_t inc_freq = 2;
	const uint9_t num_of_inc = 100;
	const uint9_t settling_cycles_number = 15;
	const SettlingTimeCyclesMultiplierOrMask settling_cycles_multiplier = SettlingTimeCyclesMultiplierOrMask::ONE_TIME;
	AD5933_Config default_config (
		command,
		range,
		pga_gain,
		sysclk_src,
		start_freq,
		inc_freq,
		num_of_inc,
		settling_cycles_number,
		settling_cycles_multiplier
	);
	return default_config;
}
