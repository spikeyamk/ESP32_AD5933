#pragma once

#include <array>
#include <bitset>
#include <cstdint>
#include <iostream>
#include <map>
#include <string>

#include "types.hpp"

enum class SYSCLK_FREQ {
	EXTERNAL = 16'000'000ul,
	INTERNAL = 16'776'000ul
};

enum class SettlingTimeCyclesMultiplierOrMask {
	ONE_TIME   = 0b0000'0000,
	TWO_TIMES  = 0b0000'0010,
	FOUR_TIMES = 0b0000'0100,
};
extern const std::map<SettlingTimeCyclesMultiplierOrMask, const char*> SettlingTimeCyclesMultiplierOrMaskStringMap;
constexpr std::bitset<8> SettlingTimeCyclesMultiplierAndMask { 0b0000'0110 };

class ControlHB {
public:
	enum class CommandOrMask {
		NOP_0 							= 0b0000'0000,
		INITIALIZE_WITH_START_FREQUENCY = 0b0001'0000,
		START_FREQUENCY_SWEEP 			= 0b0010'0000,
		INCREMENT_FREQUENCY 			= 0b0011'0000,
		REPEAT_FREQUENCY 				= 0b0100'0000,
		NOP_1 							= 0b1000'0000,
		MEASURE_TEMPERATURE 			= 0b1001'0000,
		POWER_DOWN_MODE 				= 0b1010'0000,
		STANDBY_MODE 					= 0b1011'0000,
		NOP_2 							= 0b1100'0000,
		NOP_3 							= 0b1101'0000
	}; friend std::ostream& operator<<(std::ostream& os, ControlHB::CommandOrMask value);
	static constexpr std::bitset<8> CommandAndMask { 0b1111'0000 };

	enum class OutputVoltageRangeOrMask {
		TWO_VOLT_PPK      		   = 0b0000'0000,
		ONE_VOLT_PPK      		   = 0b0000'0010,
		FOUR_HUNDRED_MILI_VOLT_PPK = 0b0000'0100,
		TWO_HUNDRED_MILI_VOLT_PPK  = 0b0000'0110
	}; friend std::ostream& operator<<(std::ostream& os, ControlHB::OutputVoltageRangeOrMask value);
	static constexpr std::bitset<8> OutputVoltageRangeAndMask { 0b0000'0110 };

	enum class PGA_GainOrMask {
		ONE_TIME   = 0b0000'0001,
		FIVE_TIMES = 0b0000'0000
	}; friend std::ostream& operator<<(std::ostream& os, ControlHB::PGA_GainOrMask value);
	static constexpr std::bitset<8> PGA_GainAndMask { 0b0000'0001 };
};

extern const std::map<ControlHB::CommandOrMask, const char*> ControlHB_CommandOrMaskStringMap;
extern const std::map<ControlHB::OutputVoltageRangeOrMask, const char*> ControlHB_OutputVoltageRangeOrMaskStringMap;
extern const std::map<ControlHB::PGA_GainOrMask, const char*> ControlHB_PGA_GainOrMaskStringMap;

class ControlLB {
public:
	static constexpr std::bitset<8> RESET_SetMask     { 0b0001'0000 };
	static constexpr std::bitset<8> RESET_ExtractMask = RESET_SetMask;

	enum class SYSCLK_SRC_OrMask {
		INTERNAL = 0b0000'0000,
		EXTERNAL = 0b0000'1000
	}; friend std::ostream& operator<<(std::ostream& os, ControlLB::SYSCLK_SRC_OrMask value);
	static constexpr std::bitset<8> SYSCLK_SRC_AndMask { 0b0000'1000 };
};
constexpr std::bitset<16> UINT9_T_ExtractMask { 0b0000'0001'1111'1111 };
extern const std::map<ControlLB::SYSCLK_SRC_OrMask, const char*> ControlLB_SYSCLK_SRC_OrMaskStringMap;

class AD5933_Config {
private:
    Register8_t control_HB;
    Register8_t control_LB;
    uint32_t active_sysclk_freq;
    Register24_t start_freq;
    Register24_t inc_freq;
    Register16_t num_of_inc;
    Register16_t settling_time_cycles;
    std::array<std::bitset<8>, 12> ad5933_config_message;
    std::array<uint8_t, 12> ad5933_config_message_raw;
    static const uint32_t pow_2_27 = 134'217'728ul;

public:
    AD5933_Config() = default;

    AD5933_Config(
        const ControlHB::CommandOrMask command,
        const ControlHB::OutputVoltageRangeOrMask range,
        const ControlHB::PGA_GainOrMask pga_gain,
        const ControlLB::SYSCLK_SRC_OrMask sysclk_src,
        const uint32_t in_start_freq,
        const uint32_t in_inc_freq,
        const uint9_t in_num_of_inc,
        const uint9_t settling_time_cycles_number,
        const SettlingTimeCyclesMultiplierOrMask settling_time_cycles_multiplier
    );

public:
    inline std::array<std::bitset<8>, 12> get_ad5933_config_message() const;
    inline std::array<uint8_t, 12> get_ad5933_config_message_raw() const;

	inline Register8_t get_control_HB(
		const Register8_t previous_state,
		const ControlHB::CommandOrMask command
	) const;

	inline Register8_t get_control_HB(
		const Register8_t previous_state,
		const ControlHB::OutputVoltageRangeOrMask range
	) const;

	inline Register8_t get_control_HB(
		const Register8_t previous_state,
		const ControlHB::PGA_GainOrMask pga_gain
	) const;

	inline Register8_t get_control_HB(
		const Register8_t previous_state,
		const ControlHB::CommandOrMask command,
		const ControlHB::OutputVoltageRangeOrMask range
	) const;

	inline Register8_t get_control_HB(
		const Register8_t previous_state,
		const ControlHB::CommandOrMask command,
		const ControlHB::PGA_GainOrMask pga_gain
	) const;

	inline Register8_t get_control_HB(
		const Register8_t previous_state,
		const ControlHB::OutputVoltageRangeOrMask range,
		const ControlHB::PGA_GainOrMask pga_gain
	) const;

	inline Register8_t get_control_HB(
		const ControlHB::CommandOrMask command,
		const ControlHB::OutputVoltageRangeOrMask range,
		const ControlHB::PGA_GainOrMask pga_gain
	) const;

    inline Register8_t get_control_LB(const ControlLB::SYSCLK_SRC_OrMask sysclk_src) const;
    inline Register8_t get_control_LB(const ControlLB::SYSCLK_SRC_OrMask sysclk_src, bool reset) const;
    inline Register8_t get_control_LB(const Register8_t previous_state, bool reset) const;
    inline Register24_t get_freq_register(const uint32_t frequency) const;
    inline Register16_t get_num_of_inc(const uint9_t in_num_of_inc) const;
    inline Register16_t get_settling_time_cycles(const Register16_t previous_state, const uint9_t number) const;
    inline Register16_t get_settling_time_cycles(const Register16_t previous_state, const SettlingTimeCyclesMultiplierOrMask multiplier) const;
    inline Register16_t get_settling_time_cycles(const uint9_t number, const SettlingTimeCyclesMultiplierOrMask multiplier) const;

	inline ControlHB::CommandOrMask get_command() const;
    inline ControlHB::OutputVoltageRangeOrMask get_voltage_range() const;
    inline ControlHB::PGA_GainOrMask get_pga_gain() const;
    inline ControlLB::SYSCLK_SRC_OrMask get_sysclk_src() const;
    inline bool get_reset() const;
    inline SettlingTimeCyclesMultiplierOrMask get_settling_time_cycles_multiplier() const;
    inline uint9_t get_uint9_t(const Register16_t state) const;
    inline uint9_t get_settling_time_cycles_number() const;
    inline uint9_t get_num_of_inc() const;
    inline double get_freq(const Register24_t state) const;
    inline double get_start_freq() const;
    inline double get_inc_freq() const;

	void set_command(ControlHB::CommandOrMask command);
	void set_voltage_range(ControlHB::OutputVoltageRangeOrMask range);
	void set_pga_gain(ControlHB::PGA_GainOrMask pga_gain);
	void set_sysclk_src(ControlLB::SYSCLK_SRC_OrMask sysclk_src);
	void set_settling_time_cycles_number(uint9_t number);
	void set_settling_time_cycles_multiplier(SettlingTimeCyclesMultiplierOrMask multiplier);
	void set_num_of_inc(uint9_t num);
	void set_start_freq(uint32_t freq);
	void set_inc_freq(uint32_t freq);

	void set_reset();

	void print() const;
};