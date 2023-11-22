#pragma once

#include <iostream>
#include <bitset>
#include <optional>
#include <vector>
#include <array>
#include <map>
#include <chrono>
#include <thread>
#include <functional>
#include <memory>
#include <cassert>
#include <cmath>
#include <atomic>

#include "util.hpp"
#include "trielo/trielo.hpp"
#include "i2c.hpp"

class AD5933 : public I2CDevice {
public:
	static const uint8_t SLAVE_ADDRESS = 0x0D;
	static const uint8_t LOWEST_REGISTER_ADDRESS = 0x80;
	static const uint8_t HIGHEST_REGISTER_ADDRESS = 0x97;
	static const int32_t pow_2_27 = 134217728ul;
	uint32_t active_sysclk_freq = static_cast<uint32_t>(Specifications::INTERNAL_SYS_CLK);

	enum CommandCodes {
		BLOCK_WRITE     = 0b1010'0000,
		BLOCK_READ      = 0b1010'0001,
		ADDRESS_POINTER = 0b1011'0000,
	};

	class RegisterAddrs {
	public:
		/* Upon powerup default ad5933->dump_all_registers():
		 * AD5933: Register[0x80]: CONTROL_HB: '0xA0'
		 * AD5933: Register[0x81]: CONTROL_LB: '0x00'
		 * AD5933: Register[0x82]: FREQ_START_HB: '0xFD'
		 * AD5933: Register[0x83]: FREQ_START_MB: '0xD7'
		 * AD5933: Register[0x84]: FREQ_START_LB: '0xED'
		 * AD5933: Register[0x85]: FREQ_INC_HB: '0xBF'
		 * AD5933: Register[0x86]: FREQ_INC_MB: '0xB7'
		 * AD5933: Register[0x87]: FREQ_INC_LB: '0xFC'
		 * AD5933: Register[0x88]: INC_NUM_HB: '0x01'
		 * AD5933: Register[0x89]: INC_NUM_LB: '0xF7'
		 * AD5933: Register[0x8A]: SETTLING_CYCLES_HB: '0x07'
		 * AD5933: Register[0x8B]: SETTLING_CYCLES_LB: '0xEB'
		 * AD5933: Register[0x8F]: STATUS: '0x70'
		 * AD5933: Register[0x92]: TEMP_DATA_HB: '0x00'
		 * AD5933: Register[0x93]: TEMP_DATA_LB: '0x00'
		 * AD5933: Register[0x94]: REAL_DATA_HB: '0x7F'
		 * AD5933: Register[0x95]: REAL_DATA_LB: '0xEF'
		 * AD5933: Register[0x96]: IMAG_DATA_HB: '0xFD'
		 * AD5933: Register[0x97]: IMAG_DATA_HB: '0xBF'
		 */

		/* Read/Write registers initial default state upon powerup:
		 * AD5933: Register[0x80]: CONTROL_HB: '0xA0': '0b1010'0000'
		       ControlHB::Commands::POWER_DOWN_MODE: '0b1010'
		       ControlHB::OutputExcitationVoltageRange::TWO_VOLT_PPK: '0b00'
			   ControlHB::PGA_Gain::FiveTimes: '0b1'

		 * AD5933: Register[0x81]: CONTROL_LB: '0x00'
		       ControlLB::SYSCLK::INT_SYSCLK: '0b0'
		       ControlLB::RESET: '0b0'

		 * AD5933: Register[0x82]: FREQ_START_HB: '0xFD'
		 * AD5933: Register[0x83]: FREQ_START_MB: '0xD7'
		 * AD5933: Register[0x84]: FREQ_START_LB: '0xED'
		 *     FREQ_START == around 516'456 Hz
		 * 
		 * AD5933: Register[0x85]: FREQ_INC_HB: '0xBF'
		 * AD5933: Register[0x86]: FREQ_INC_MB: '0xB7'
		 * AD5933: Register[0x87]: FREQ_INC_LB: '0xFC'
		 *     FREQ_INC == around 390'060 Hz
		 * 
		 * AD5933: Register[0x88]: INC_NUM_HB: '0x01'
		 * AD5933: Register[0x89]: INC_NUM_LB: '0xF7'
		 *     INC_NUM == 503 times
		 * 
		 * AD5933: Register[0x8A]: SETTLING_CYCLES_HB: '0x07'
		 *     SettlingCyclesHB::Multiplier::FOUR_TIMES: '0b11'
		 * AD5933: Register[0x8B]: SETTLING_CYCLES_LB: '0xEB'
		 *     Settling Cycles == 491 times
		 * 
		 * Special STATUS register initial default state upon powerup:
		 * AD5933: Register[0x8F]: STATUS: '0x70'
		 *     TEMP_DATA_VALID == 0b0 == false
		 *     REAL_DATA_VALID == 0b0 == false
		 *     IMAG_DATA_VALID == 0b0 == false
		 * 
		 * Read-only registers initial default state upon powerup:
		 * AD5933: Register[0x92]: TEMP_DATA_HB: '0x00'
		 * AD5933: Register[0x93]: TEMP_DATA_LB: '0x00'
		 * AD5933: Register[0x94]: REAL_DATA_HB: '0x7F'
		 * AD5933: Register[0x95]: REAL_DATA_LB: '0xEF'
		 * AD5933: Register[0x96]: IMAG_DATA_HB: '0xFD'
		 * AD5933: Register[0x97]: IMAG_DATA_HB: '0xBF'
		 */

		/* Read/Write Register addresses */
		static constexpr RegisterAddr_RW CONTROL_HB			{ 0x80 };
		static constexpr RegisterAddr_RW CONTROL_LB		    { 0x81 }; // Done
		static constexpr RegisterAddr_RW FREQ_START_HB      { 0x82 }; // Done
		static constexpr RegisterAddr_RW FREQ_START_MB      { 0x83 }; // Done
		static constexpr RegisterAddr_RW FREQ_START_LB      { 0x84 }; // Done
		static constexpr RegisterAddr_RW FREQ_INC_HB        { 0x85 }; // Done
		static constexpr RegisterAddr_RW FREQ_INC_MB        { 0x86 }; // Done
		static constexpr RegisterAddr_RW FREQ_INC_LB        { 0x87 }; // Done
		static constexpr RegisterAddr_RW INC_NUM_HB         { 0x88 }; // Done
		static constexpr RegisterAddr_RW INC_NUM_LB         { 0x89 }; // Done
		static constexpr RegisterAddr_RW SETTLING_CYCLES_HB { 0x8A }; // Done
		static constexpr RegisterAddr_RW SETTLING_CYCLES_LB { 0x8B }; // Done

		/* Read-only Register addresses */
		static constexpr RegisterAddr_RO STATUS { 0x8F };

		static constexpr RegisterAddr_RO TEMP_DATA_HB { 0x92 };
		static constexpr RegisterAddr_RO TEMP_DATA_LB { 0x93 };
		static constexpr RegisterAddr_RO REAL_DATA_HB { 0x94 };
		static constexpr RegisterAddr_RO REAL_DATA_LB { 0x95 };
		static constexpr RegisterAddr_RO IMAG_DATA_HB { 0x96 };
		static constexpr RegisterAddr_RO IMAG_DATA_LB { 0x97 };

		/* All valid Read/Write addresses array */
		static constexpr std::array<RegisterAddr, 12> valid_read_write {
			CONTROL_HB, CONTROL_LB, FREQ_START_HB,
			FREQ_START_MB, FREQ_START_LB, FREQ_INC_HB,
			FREQ_INC_MB, FREQ_INC_LB, INC_NUM_HB,
			INC_NUM_LB, SETTLING_CYCLES_HB, SETTLING_CYCLES_LB
		};

		/* All valid Read-only addresses array */
		static constexpr std::array<RegisterAddr, 7> valid_read_only {
			STATUS, TEMP_DATA_HB, TEMP_DATA_LB,
			REAL_DATA_HB, REAL_DATA_LB, IMAG_DATA_HB,
			IMAG_DATA_LB
		};

		static bool is_valid_read_write_register_addr(const RegisterAddr addr_to_check) {
			for(const auto i: valid_read_write) {
				if(i == addr_to_check) {
					return true;
				}
			}
			return false;
		}

		static bool is_valid_read_only_register_addr(const RegisterAddr addr_to_check) {
			for(const auto i: valid_read_only) {
				if(i == addr_to_check) {
					return true;
				}
			}
			return false;
		}

		static bool is_valid_register_addr(const RegisterAddr addr_to_check) {
			if(is_valid_read_only_register_addr(addr_to_check) 
			|| is_valid_read_write_register_addr(addr_to_check)) {
				return true;
			} else {
				return false;
			}
		}

		enum class RW {
			CONTROL_HB		   = 0x80,
			CONTROL_LB		   = 0x81,
			FREQ_START_HB      = 0x82,   // Start frequency
			FREQ_START_MB      = 0x83,   // Start frequency
			FREQ_START_LB      = 0x84,   // Start frequency
			FREQ_INC_HB        = 0x85,   // Frequency increment
			FREQ_INC_MB        = 0x86,   // Frequency increment
			FREQ_INC_LB        = 0x87,   // Frequency increment
			INC_NUM_HB         = 0x88,   // Number of increments
			INC_NUM_LB         = 0x89,   // Number of increments
			SETTLING_CYCLES_HB = 0x8A,   // Number of settling time cycles
			SETTLING_CYCLES_LB = 0x8B,   // Number of settling time cycles
		};

		enum class RO {
			STATUS			   = 0x8F,
			TEMP_DATA_HB	   = 0x92,   // Temperature data
			TEMP_DATA_LB	   = 0x93,
			REAL_DATA_HB       = 0x94,   // Real data
			REAL_DATA_LB       = 0x95,   // Real data
			IMAG_DATA_HB       = 0x96,   // Imaginary data
			IMAG_DATA_LB       = 0x97    // Imaginary data
		};
	};

	class ControlHB {
	public:
		enum class PGA_Gain {
			FiveTimes = 0,
			OneTime = 1
		};

		enum class OutputExcitationVoltageRange {
			TWO_VOLT_PPK      		   = 0b00 << 1,
			ONE_VOLT_PPK      		   = 0b01 << 1,
			FOUR_HUNDRED_MILI_VOLT_PPK = 0b10 << 1,
			TWO_HUNDRED_MILI_VOLT_PPK  = 0b11 << 1,
		};

		enum class Commands {
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
		};

		static constexpr std::array<uint8_t, 11> CommandsArray {
			static_cast<uint8_t>(Commands::NOP_0),
			static_cast<uint8_t>(Commands::INITIALIZE_WITH_START_FREQUENCY),
			static_cast<uint8_t>(Commands::START_FREQUENCY_SWEEP),
			static_cast<uint8_t>(Commands::INCREMENT_FREQUENCY),
			static_cast<uint8_t>(Commands::REPEAT_FREQUENCY),
			static_cast<uint8_t>(Commands::NOP_1),
			static_cast<uint8_t>(Commands::MEASURE_TEMPERATURE),
			static_cast<uint8_t>(Commands::POWER_DOWN_MODE),
			static_cast<uint8_t>(Commands::STANDBY_MODE),
			static_cast<uint8_t>(Commands::NOP_2),
			static_cast<uint8_t>(Commands::NOP_3)
		};
	};

	class ControlLB {
	public:
		/* D7 Reserved; set to 0
		 * D6 Reserved; set to 0
		 * D5 Reserved; set to 0
		 * D4 Reset
		 * D3 External system clock; set to 1
		 * Internal system clock; set to 0
		 * D2 Reserved; set to 0
		 * D1 Reserved; set to 0
		 * D0 Reserved; set to 0 */
		enum class Command {
			RESET      = (0x1 << 4)
		};

		enum class SYSCLK_SRC {
			INT_SYSCLK = (0x0 << 3),
			EXT_SYSCLK = (0x1 << 3)
		};
	};


	class SettlingCyclesHB {
	public:
		enum class Multiplier {
			ONE_TIME   = ( 0b00 << 1),
			TWO_TIMES  = ( 0b01 << 1),
			FOUR_TIMES = ( 0b11 << 1),
		};

	};

	enum class Status {
		/*
		* 0x8A D15 to D11 Don’t care Read or write Integer number stored in
		* binary formatD10 to D9 2-bit decode
		* D10 D9 Description
		* 0 0 Default
		* 0 1 No. of cycles × 2
		* 1 0 Reserved
		* 1 1 No. of cycles × 4
		* D8 MSB number of settling time cycles */
		TEMP_VALID = (0x1 << 0),
		IMPEDANCE_VALID = (0x1 << 1),
		SWEEP_DONE = (0x1 << 2)
	};

	enum class Specifications {
		EXTERNAL_SYS_CLK = 16000000ul,
		INTERNAL_SYS_CLK = 16776000ul,
		MAX_INC_NUM      = 511,
		MAX_CYCLES_NUM   = 511
	};

	AD5933(const i2c_master_dev_handle_t &device_handle) :
		I2CDevice(device_handle, LOWEST_REGISTER_ADDRESS, HIGHEST_REGISTER_ADDRESS)
	{}

	bool write_to_register_address_pointer(const RegisterAddr register_address) const {
		if(RegisterAddrs::is_valid_register_addr(register_address) == false) {
			return false;
		}
		uint8_t write_buffer[2] { ADDRESS_POINTER, register_address.unwrap() };
		const int xfer_timeout_ms = -1;
		ESP_ERROR_CHECK(
			i2c_master_transmit(
				device_handle,
				write_buffer,
				sizeof(write_buffer),
				xfer_timeout_ms
			)
		);
		return true;
	}

	void dump_all_registers() const override {
		std::printf("\nAD5933: Dumping all registers\n");
		const auto rw_ret = block_read_register(
			AD5933::RegisterAddrs::CONTROL_HB,
			AD5933::RegisterAddrs::SETTLING_CYCLES_LB.unwrap() - AD5933::RegisterAddrs::CONTROL_HB.unwrap() + 1
		);
		if(rw_ret.has_value()) {
			for(uint8_t i = 0; i < rw_ret.value().size(); i++) {
				std::printf("AD5933: Register[0x%02X]: 0x%02X\n", AD5933::RegisterAddrs::CONTROL_HB.unwrap() + i, rw_ret.value()[i]);
			}
		}
		const auto status_ret = read_register(AD5933::RegisterAddrs::STATUS);
		if(status_ret.has_value()) {
			std::printf("AD5933: Register[0x%02X]: 0x%02X\n", AD5933::RegisterAddrs::STATUS.unwrap(), status_ret.value());
		}
		const auto ro_ret = block_read_register(
			AD5933::RegisterAddrs::TEMP_DATA_HB,
			AD5933::RegisterAddrs::IMAG_DATA_LB.unwrap() - AD5933::RegisterAddrs::TEMP_DATA_HB.unwrap() + 1
		);
		if(ro_ret.has_value()) {
			for(uint8_t i = 0; i < ro_ret.value().size(); i++) {
				std::printf("AD5933: Register[0x%02X]: 0x%02X\n", AD5933::RegisterAddrs::TEMP_DATA_HB.unwrap() + i, ro_ret.value()[i]);
			}
		}
		std::printf("\n");
	}

	std::optional<std::array<uint8_t, 19>> dump_all_registers_as_array() {
		std::printf("\nAD5933: Dumping all registers\n");
		const auto rw_ret = block_read_register(
			AD5933::RegisterAddrs::CONTROL_HB,
			AD5933::RegisterAddrs::SETTLING_CYCLES_LB.unwrap() - AD5933::RegisterAddrs::CONTROL_HB.unwrap() + 1
		);
		if(rw_ret.has_value()) {
			for(uint8_t i = 0; i < rw_ret.value().size(); i++) {
				std::printf("AD5933: Register[0x%02X]: 0x%02X\n", AD5933::RegisterAddrs::CONTROL_HB.unwrap() + i, rw_ret.value()[i]);
			}
		} else {
			return std::nullopt;
		}
		const auto status_ret = read_register(AD5933::RegisterAddrs::STATUS);
		if(status_ret.has_value()) {
			std::printf("AD5933: Register[0x%02X]: 0x%02X\n", AD5933::RegisterAddrs::STATUS.unwrap(), status_ret.value());
		} else {
			return std::nullopt;
		}
		const auto ro_ret = block_read_register(
			AD5933::RegisterAddrs::TEMP_DATA_HB,
			AD5933::RegisterAddrs::IMAG_DATA_LB.unwrap() - AD5933::RegisterAddrs::TEMP_DATA_HB.unwrap() + 1
		);
		if(ro_ret.has_value()) {
			for(uint8_t i = 0; i < ro_ret.value().size(); i++) {
				std::printf("AD5933: Register[0x%02X]: 0x%02X\n", AD5933::RegisterAddrs::TEMP_DATA_HB.unwrap() + i, ro_ret.value()[i]);
			}
		} else {
			return std::nullopt;
		}
		std::printf("\n");

		std::vector<uint8_t> ret_vector { rw_ret.value().begin(), rw_ret.value().end() };
		ret_vector.push_back(status_ret.value());
		ret_vector.insert(ret_vector.end(), ro_ret.value().begin(), ro_ret.value().end());
		std::array<uint8_t, 19> ret_array;
		std::copy(ret_vector.begin(), ret_vector.end(), ret_array.begin());
		return ret_array;
	}


	template<typename T_Collection>
	bool block_write_to_register(const RegisterAddr_RW &register_address, const T_Collection &message) const {
		write_to_register_address_pointer(register_address);
		std::vector<uint8_t> write_buffer { 
			static_cast<uint8_t>(AD5933::CommandCodes::BLOCK_WRITE),
			static_cast<uint8_t>(message.size())
		};
		write_buffer.insert(write_buffer.end(), message.begin(), message.end());
		const int xfer_timeout_ms = -1;
		ESP_ERROR_CHECK(
			i2c_master_transmit(
				device_handle,
				write_buffer.data(),
				write_buffer.size(),
				xfer_timeout_ms 
			)
		);

		const std::optional<std::vector<uint8_t>> values_to_check_against = block_read_register(register_address, message.size());
		if(values_to_check_against.has_value() == false) {
			std::printf("AD5933: Failed to read the block at the register address: %u\n", register_address.unwrap());
			return false;
		}

    	if (std::equal(message.begin(), message.end(), values_to_check_against.value().begin(), values_to_check_against.value().end())) {
			return true;
		} else {
			std::printf(
				"AD5933: Failed to verify the written block at address: 0x%02X with message:\n",
				register_address.unwrap()
			);
			for(size_t i = 0; i < message.size(); i++) {
				std::printf("\tAD5933: message[%u]: 0x%02X\n", i, message[i]);
			}

			for(size_t i = 0; i < values_to_check_against.value().size(); i++) {
				std::printf("\tAD5933: values_to_check_against[%u]: 0x%02X\n", i, values_to_check_against.value()[i]);
			}
			std::printf("\n");
			return false;
		}
	}

	std::optional<uint8_t> read_register(const RegisterAddr register_address) const override {
		write_to_register_address_pointer(register_address);
		uint8_t read_buffer;	
		const int xfer_timeout_ms = -1;
		ESP_ERROR_CHECK(
			i2c_master_receive(
				device_handle,
				&read_buffer,
				sizeof(read_buffer),
				xfer_timeout_ms
			)
		);
		return std::optional<uint8_t> { read_buffer };
	}

	std::optional<std::vector<uint8_t>> block_read_register(
		const RegisterAddr register_starting_address,
		const uint8_t n_bytes
	) const override {
		if(register_address_range_exists(register_starting_address, n_bytes) == false) {
			return std::nullopt;
		}

		const int xfer_timeout_ms = -1;
		std::vector<uint8_t> output_vector;

		const uint8_t block_size = 10;
		for(uint8_t i = 0; i < n_bytes; i += block_size ) {
			write_to_register_address_pointer(RegisterAddr{ static_cast<uint8_t>(register_starting_address.unwrap() + i) });
			uint8_t write_buffer[2] = { AD5933::CommandCodes::BLOCK_READ, n_bytes };
			uint8_t read_buffer[block_size] = {
				0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00
			};

			uint8_t remaining = n_bytes - i;
			ESP_ERROR_CHECK(
				i2c_master_transmit_receive(
					device_handle,
					write_buffer,
					sizeof(write_buffer),
					read_buffer,
					std::min(static_cast<uint8_t>(block_size), remaining), // Ensure not to read more than remaining data.
					xfer_timeout_ms
				)
			);

			for(uint8_t j = 0; j < std::min(static_cast<uint8_t>(block_size), remaining); ++j) {
				output_vector.push_back(read_buffer[j]);
			}
		}

		return std::optional<std::vector<uint8_t>>(output_vector);
	}

	std::optional<ControlHB::Commands> read_control_mode() {
		const std::optional<uint8_t> control_hb_state = read_register(RegisterAddrs::CONTROL_HB);
		if(control_hb_state.has_value() == false) {
			std::printf("AD5933: Failed to read the control mode:\nFailed to read the CONTROL_HB register\n");
			return std::nullopt;
		}

		for(const uint8_t command: ControlHB::CommandsArray) {
			if((control_hb_state.value() & 0b1111'0000) == command) {
				return std::optional<ControlHB::Commands> { ControlHB::Commands { command } };
			}
		}

		return std::nullopt;
	}

	bool reset() {
		const std::optional<uint8_t> control_lb_state_before_reset { read_register(RegisterAddrs::CONTROL_LB).value() };
		if(control_lb_state_before_reset.has_value() == false) {
			std::printf("AD5933: Reset: Failed to read the CONTROL_LB register\n");
			return false;
		}
		
		const uint8_t reset_command = (
			control_lb_state_before_reset.value()
			| static_cast<uint8_t>(AD5933::ControlLB::Command::RESET)
		);
		write_to_register_wo_check(RegisterAddrs::CONTROL_LB, reset_command);

		const std::optional<uint8_t> control_lb_state_after_reset = read_register(RegisterAddrs::CONTROL_LB);
		if(control_lb_state_after_reset.has_value() == false) {
			std::printf("AD5933: Reset: Failed to read the CONTROL_LB register\n");
			return false;
		}

		if(control_lb_state_before_reset.value() != control_lb_state_after_reset.value()) {
			std::printf("AD5933: Reset: Failed verify the CONTROL_LB register before and after reset\n");
			return false;
		}

		const std::optional<ControlHB::Commands> control_hb_mode_after_reset = read_control_mode();
		if(control_hb_mode_after_reset.has_value() == false) {
			std::printf("AD5933: Reset: Failed to read the CONTROL_HB mode\n");
			return false;
		}

		if(control_hb_mode_after_reset.value() == ControlHB::Commands::STANDBY_MODE) {
			std::printf("AD5933: Reset: Failed verify the CONTROL_HB mode to STANDBY_MODE after reset\n");
			return false;
		}

		return true;
	}

	bool set_sysclk_src(const ControlLB::SYSCLK_SRC clk_src) {
		const std::optional<uint8_t> control_lb_state_before_set_sysclk { read_register(RegisterAddrs::CONTROL_LB).value() };
		if(control_lb_state_before_set_sysclk.has_value() == false) {
			std::printf("AD5933: Reset: Failed to read the CONTROL_LB register\n");
			return false;
		}

		uint8_t sysclk_message;
		switch(static_cast<uint8_t>(clk_src)) {
			default:
				sysclk_message = (
					(control_lb_state_before_set_sysclk.value() & 0b1111'01111)
					| static_cast<uint8_t>(ControlLB::SYSCLK_SRC::INT_SYSCLK)
				);
				active_sysclk_freq = static_cast<uint32_t>(Specifications::INTERNAL_SYS_CLK);
				break;
			case static_cast<uint8_t>(ControlLB::SYSCLK_SRC::EXT_SYSCLK):
				sysclk_message = (
					(control_lb_state_before_set_sysclk.value() & 0b1111'01111)
					| static_cast<uint8_t>(ControlLB::SYSCLK_SRC::EXT_SYSCLK)
				);
				active_sysclk_freq = static_cast<uint32_t>(Specifications::EXTERNAL_SYS_CLK);
				break;
		}

		if(write_to_register(RegisterAddrs::CONTROL_LB, sysclk_message) == false) {
			std::printf("AD5933: Failed to set the SYS_CLK by writing to the CONTROL_LB register\n");
			return false;
		} else {
			return true;
		}
	}

	bool set_pga_gain(const ControlHB::PGA_Gain pga_gain) {
		const std::optional<uint8_t> control_hb_state_before_pga_gain { read_register(RegisterAddrs::CONTROL_LB).value() };
		if(control_hb_state_before_pga_gain.has_value() == false) {
			std::printf("AD5933: Reset: Failed to read the CONTROL_HB register\n");
			return false;
		}

		uint8_t pga_gain_message = (control_hb_state_before_pga_gain.value() & 0b1111'1110);
		switch(static_cast<uint8_t>(pga_gain)) {
			default:
				pga_gain_message |= static_cast<uint8_t>(ControlHB::PGA_Gain::OneTime);
				break;
			case static_cast<uint8_t>(ControlHB::PGA_Gain::FiveTimes):
				pga_gain_message |= static_cast<uint8_t>(ControlHB::PGA_Gain::FiveTimes);
				break;
		}

		if(write_to_register(RegisterAddrs::CONTROL_HB, pga_gain_message) == false) {
			std::printf("AD5933: Failed to set the PGA Gain by writing to the CONTROL_HB register\n");
			return false;
		} else {
			return true;
		}
	}

	bool set_output_excitation_voltage_range(const ControlHB::OutputExcitationVoltageRange range) {
		const std::optional<uint8_t> control_hb_state_before_range { read_register(RegisterAddrs::CONTROL_LB).value() };
		if(control_hb_state_before_range.has_value() == false) {
			std::printf("AD5933: Reset: Failed to read the CONTROL_HB register\n");
			return false;
		}

		uint8_t range_message = (control_hb_state_before_range.value() & 0b1111'1001);
		switch(static_cast<uint8_t>(range)) {
			default:
				range_message |= static_cast<uint8_t>(ControlHB::OutputExcitationVoltageRange::TWO_VOLT_PPK);
				break;
			case static_cast<uint8_t>(ControlHB::OutputExcitationVoltageRange::ONE_VOLT_PPK):
				range_message |= static_cast<uint8_t>(ControlHB::OutputExcitationVoltageRange::ONE_VOLT_PPK);
				break;
			case static_cast<uint8_t>(ControlHB::OutputExcitationVoltageRange::FOUR_HUNDRED_MILI_VOLT_PPK):
				range_message |= static_cast<uint8_t>(ControlHB::OutputExcitationVoltageRange::FOUR_HUNDRED_MILI_VOLT_PPK);
				break;
			case static_cast<uint8_t>(ControlHB::OutputExcitationVoltageRange::TWO_HUNDRED_MILI_VOLT_PPK):
				range_message |= static_cast<uint8_t>(ControlHB::OutputExcitationVoltageRange::TWO_HUNDRED_MILI_VOLT_PPK);
				break;
		}

		if(write_to_register(RegisterAddrs::CONTROL_HB, range_message) == false) {
			std::printf("AD5933: Failed to set the Output Excitation Voltage by writing to the CONTROL_HB register\n");
			return false;
		} else {
			return true;
		}
	}

	bool set_range_and_gain(const ControlHB::OutputExcitationVoltageRange range, const ControlHB::PGA_Gain gain) {
		if(set_output_excitation_voltage_range(range) == false) {
			std::printf("AD5933: Set Range and Gain failed: Failed to set the Output Excitation Voltage Range\n");
			return false;
		}

		if(set_pga_gain(gain) == false) {
			std::printf("AD5933: Set Range and Gain failed: Failed to set the PGA Gain\n");
			return false;
		}

		return true;
	}

	bool set_control_command(const ControlHB::Commands command) {
		const std::optional<uint8_t> control_hb_state_before_command { read_register(RegisterAddrs::CONTROL_LB).value() };
		if(control_hb_state_before_command.has_value() == false) {
			std::printf("AD5933: Reset: Failed to read the CONTROL_HB register\n");
			return false;
		}

		uint8_t command_message = (control_hb_state_before_command.value() & 0b0000'1111);
		switch(static_cast<uint8_t>(command)) {
			default:
				command_message |= static_cast<uint8_t>(ControlHB::Commands::NOP_0);
				break;
			case static_cast<uint8_t>(ControlHB::Commands::INITIALIZE_WITH_START_FREQUENCY):
				command_message |= static_cast<uint8_t>(ControlHB::Commands::INITIALIZE_WITH_START_FREQUENCY);
				break;
			case static_cast<uint8_t>(ControlHB::Commands::START_FREQUENCY_SWEEP):
				command_message |= static_cast<uint8_t>(ControlHB::Commands::START_FREQUENCY_SWEEP);
				break;
			case static_cast<uint8_t>(ControlHB::Commands::INCREMENT_FREQUENCY):
				command_message |= static_cast<uint8_t>(ControlHB::Commands::INCREMENT_FREQUENCY);
				break;
			case static_cast<uint8_t>(ControlHB::Commands::REPEAT_FREQUENCY):
				command_message |= static_cast<uint8_t>(ControlHB::Commands::REPEAT_FREQUENCY);
				break;
			case static_cast<uint8_t>(ControlHB::Commands::MEASURE_TEMPERATURE):
				command_message |= static_cast<uint8_t>(ControlHB::Commands::MEASURE_TEMPERATURE);
				break;
			case static_cast<uint8_t>(ControlHB::Commands::POWER_DOWN_MODE):
				command_message |= static_cast<uint8_t>(ControlHB::Commands::POWER_DOWN_MODE);
				break;
			case static_cast<uint8_t>(ControlHB::Commands::STANDBY_MODE):
				command_message |= static_cast<uint8_t>(ControlHB::Commands::STANDBY_MODE);
				break;
		}

		if(write_to_register(RegisterAddrs::CONTROL_HB, command_message) == false) {
			std::printf("AD5933: Failed to set the Output Excitation Voltage by writing to the CONTROL_HB register\n");
			return false;
		} else {
			return true;
		}
	}

	std::optional<std::array<uint8_t, 3>> convert_frequency_to_binary_coded_frequency_message(const uint32_t frequency) {
		const uint32_t binary_coded_frequency = (uint32_t)((double)frequency * 4 /
			static_cast<double>(active_sysclk_freq) * pow_2_27);
		const uint32_t twenty_four_bit_max_number = 0xFFFFFF;

		if(binary_coded_frequency > twenty_four_bit_max_number) {
			std::printf("AD5933: Overflow: Failed to convert frequency to binary coded frequency message from frequency: %lu\n", frequency);
			return std::nullopt;
		} else {
			return std::optional<std::array<uint8_t, 3>> {
				std::array<uint8_t, 3> {
					(uint8_t) ((binary_coded_frequency  >> (2 * 8)) & 0xFF),
					(uint8_t) ((binary_coded_frequency  >> (1 * 8)) & 0xFF),
					(uint8_t) ((binary_coded_frequency  >> (0 * 8)) & 0xFF)
				}
			};
		}
	}

	std::optional<uint32_t> convert_binary_coded_frequency_message_to_frequency(const std::array<uint8_t, 3> binary_coded_frequency_message) { 
		const uint32_t binary_coded_frequency = (
		    (static_cast<uint32_t>(binary_coded_frequency_message[0]) << (2 * 8)) |
		    (static_cast<uint32_t>(binary_coded_frequency_message[1]) << (1 * 8)) |
		    (static_cast<uint32_t>(binary_coded_frequency_message[2]) << (0 * 8))
		);

		const uint32_t twenty_four_bit_max_number = 0xFFFFFF;
		if(binary_coded_frequency > twenty_four_bit_max_number) {
			std::printf("AD5933: Overflow: Failed to convert binary coded frequency message to frequency from: 0x%02X%02X%02X\n", 
				binary_coded_frequency_message[0],
				binary_coded_frequency_message[1],
				binary_coded_frequency_message[2]
			);
			return std::nullopt;
		}
		uint32_t frequency = static_cast<uint32_t>(
			(
				(static_cast<double>(binary_coded_frequency) / 4.00f) *
				static_cast<double>(active_sysclk_freq)
			) / static_cast<double>(pow_2_27)
		);
		return std::optional<uint32_t> { frequency };
	}

	bool set_freq(const RegisterAddr_RW freq_register, const uint32_t freq) {
		assert(
			(freq_register == RegisterAddrs::FREQ_START_HB) ||
			(freq_register == RegisterAddrs::FREQ_INC_HB)
		);
		const std::optional<std::array<uint8_t, 3>> frequency_message = convert_frequency_to_binary_coded_frequency_message(freq);
		if(frequency_message.has_value() == false) {
			std::printf("AD5933: Failed to set frequency to: %lu\n", freq);
			return false;
		}

		if(block_write_to_register(freq_register, frequency_message.value()) == false) {
			std::printf("AD5933: Failed to write the frequency message to frequency register[%u]: %lu\n",
				freq_register.unwrap(),
				freq
			);
			return false;
		} else {
			return true;
		}
	}

	bool set_start_freq(const uint32_t freq) {
		if(set_freq(RegisterAddrs::FREQ_START_HB, freq) == true) {
			return true;
		} else {
			std::printf("AD5933: Failed to set the start frequency to: %lu\n", freq);
			return false;
		}
	}

	bool set_inc_freq(const uint32_t freq) {
		if(set_freq(RegisterAddrs::FREQ_INC_HB, freq) == true) {
			return true;
		} else {
			std::printf("AD5933: Failed to set the increment frequency to: %lu\n", freq);
			return false;
		}
	}

	bool set_inc_num(const uint16_t inc_num) {
		if(inc_num > static_cast<uint16_t>(Specifications::MAX_INC_NUM)) {
			std::printf("AD5933: Overflow: Failed to set the increment number to: %u\n", inc_num);
			std::printf("AD5933: Maximum increment number: %u\n", static_cast<uint16_t>(Specifications::MAX_INC_NUM));
			return false;
		}

		const std::array<uint8_t, 2> inc_num_message {
			(uint8_t) ((inc_num >> (1 * 8)) & 0xFF),
			(uint8_t) ((inc_num >> (0 * 8)) & 0xFF)
		};

		if(block_write_to_register(RegisterAddrs::INC_NUM_HB, inc_num_message) == false) {
			std::printf("AD5933: Failed to write the increment number: %u to the registers\n", inc_num);
			return false;
		} else {
			return true;
		}
	}

	bool configure_sweep(const uint32_t start_freq,
						 const uint32_t inc_freq,
						 const uint16_t inc_num) {
		if(set_start_freq(start_freq) == false) {
			std::printf("AD5933: Failed to configure sweep: Failed to set the start frequency to: %lu\n", start_freq);
			return false;
		}

		if(set_inc_freq(inc_freq) == false) {
			std::printf("AD5933: Failed to configure sweep: Failed to set the increment frequency to: %lu\n", inc_freq);
			return false;
		}

		if(set_inc_num(inc_num) == false) {
			std::printf("AD5933: Failed to configure sweep: Failed to set the increment number to: %u\n", inc_num);
			return false;
		}

		return true;
	}

	bool set_settling_time_multiplier(const SettlingCyclesHB::Multiplier multiplier) {
		const std::optional<uint8_t> settling_cycles_hb_before_transmit = read_register(RegisterAddrs::SETTLING_CYCLES_HB);
		if(settling_cycles_hb_before_transmit.has_value() == false) {
			std::printf("AD5933: Failed to set the Time settling multiplier:\n");
			std::printf("Failed to read the SETTLING_CYCLES_HB register\n");
			return false;
		}

		uint8_t message;
		switch(static_cast<uint8_t>(multiplier)) {
			default:
				message = (
					(settling_cycles_hb_before_transmit.value() & 0b1111'1001)
					| static_cast<uint8_t>(SettlingCyclesHB::Multiplier::ONE_TIME)
				);
				break;
			case static_cast<uint8_t>(SettlingCyclesHB::Multiplier::TWO_TIMES):
				message = (
					(settling_cycles_hb_before_transmit.value() & 0b1111'1001)
					| static_cast<uint8_t>(SettlingCyclesHB::Multiplier::TWO_TIMES)
				);
				break;
			case static_cast<uint8_t>(SettlingCyclesHB::Multiplier::FOUR_TIMES):
				message = (
					(settling_cycles_hb_before_transmit.value() & 0b1111'1001)
					| static_cast<uint8_t>(SettlingCyclesHB::Multiplier::FOUR_TIMES)
				);
				break;
		}

		if(write_to_register(RegisterAddrs::SETTLING_CYCLES_HB, message) == false) {
			std::printf("AD5933: Failed to set the Time settling multiplier:\n");
			std::printf("AD5933: Failed to write to the SETTLING_CYCLES_HB register\n");
			return false;
		}

		return true;
	}

	bool set_settling_time_cycles_number(const uint16_t cycles_num) {
		if(cycles_num > static_cast<uint16_t>(Specifications::MAX_CYCLES_NUM)) {
			std::printf("AD5933: Overflow: Failed to set the settling time cycles number to: %u\n", cycles_num);
			std::printf("AD5933: Maximum settling time cycles number: %u\n", static_cast<uint16_t>(Specifications::MAX_CYCLES_NUM));
			return false;
		}

		const std::optional<uint8_t> settling_cycles_hb_before_transmit = read_register(RegisterAddrs::SETTLING_CYCLES_HB);
		if(settling_cycles_hb_before_transmit.has_value() == false) {
			std::printf("AD5933: Failed to set the settling time cycles number:\n");
			std::printf("Failed to read the SETTLING_CYCLES_HB register\n");
			return false;
		}

		const uint8_t cycles_num_message_hb = (
			(settling_cycles_hb_before_transmit.value() & 0b1111'1110)
			| static_cast<uint8_t>((cycles_num >> (1 * 8)) & 0xFF)
		);
		const uint8_t cycles_num_message_lb = (uint8_t) ((cycles_num >> (0 * 8)) & 0xFF);

		std::array<uint8_t, 2> cycles_num_message {
			cycles_num_message_hb,
			cycles_num_message_lb
		};

		if(block_write_to_register(RegisterAddrs::SETTLING_CYCLES_HB, cycles_num_message) == false) {
			std::printf("AD5933: Failed to set the settling time cycles number:\n");
			std::printf("AD5933: Failed to write to the SETTLING_CYCLES_HB and SETTLING_CYCLES_LB register\n");
			return false;
		}
		return true;
	}

public:
	std::optional<float> measure_temperature() {
		if(set_control_command(ControlHB::Commands::MEASURE_TEMPERATURE) == false) {
			std::printf("AD5933: Measure Temperature failed: Failed to write the MEASURE_TEMPERATURE command to CONTROL_HB register\n");
			return std::nullopt;
		}
		while(has_status_condition(Status::TEMP_VALID) == false) {
			std::printf("AD5933: Measure Temperature: Waiting for TEMP_VALID in the STATUS register\n");
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}

		const std::optional<std::vector<uint8_t>> temp_data = block_read_register(RegisterAddrs::TEMP_DATA_HB, 2);
		if(temp_data.has_value() == false) {
			std::printf("AD5933: Measure Temperature failed to read the TEMP_DATA_HB and TEMP_DATA_LB registers\n");
			return std::nullopt;
		}

		const uint8_t temp_hb = temp_data.value()[0];
		const uint8_t temp_lb = temp_data.value()[1];

		uint16_t temperature = 0;
		temperature |= temp_lb;
		temperature |= (temp_hb << 8);

	    if(temperature < 8192) {
	    	return std::optional<float> { (static_cast<float>(temperature) / 32.0f) };
	    } else {
	    	return std::optional<float> { (static_cast<float>((temperature - 16384)) / 32.0f) };
	    }
	}

	bool has_status_condition(const Status status_condition) {
		const std::optional<uint8_t> status_state = read_register(RegisterAddrs::STATUS);
		if(status_state.has_value() == false) {
			return false;
		}

		if((status_state.value() & static_cast<uint8_t>(status_condition)) == 
		static_cast<uint8_t>(status_condition)) {
			return true;
		} else {
			return false;
		}
	}

	std::optional<SettlingCyclesHB::Multiplier> read_settling_time_cycles_multiplier() {
		const std::optional<uint8_t> settling_cycles_HB_state = read_register(RegisterAddrs::SETTLING_CYCLES_HB);
		if(settling_cycles_HB_state.has_value() == false) {
			std::printf("AD5933: Failed to read the settling cycles multiplier: Failed to read the SETTLING_CYCLES_HB register\n");
			return std::nullopt;
		}

		const uint8_t settling_cycles_multiplier_bits = (settling_cycles_HB_state.value() && 0b0000'0110);
		switch(settling_cycles_multiplier_bits) {
			default:
				return AD5933::SettlingCyclesHB::Multiplier::ONE_TIME;
			case static_cast<uint8_t>(AD5933::SettlingCyclesHB::Multiplier::TWO_TIMES):
				return AD5933::SettlingCyclesHB::Multiplier::TWO_TIMES;
			case static_cast<uint8_t>(AD5933::SettlingCyclesHB::Multiplier::FOUR_TIMES):
				return AD5933::SettlingCyclesHB::Multiplier::FOUR_TIMES;
		}
	}

	std::optional<uint16_t> read_settling_time_cycles() {
		const size_t HB_index = 0;
		const size_t LB_index = 1;
		const std::optional<std::vector<uint8_t>> settling_cycles_HB_LB_state = block_read_register(RegisterAddrs::SETTLING_CYCLES_HB, 2);
		if(settling_cycles_HB_LB_state.has_value() == false) {
			std::printf("AD5933: Failed to read the settling cycles multiplier:\n\tAD5933: Failed to read the SETTLING_CYCLES_HB and SETTLING_CYCLES_LB registers\n");
			return std::nullopt;
		}

		const uint16_t settling_time_cycles = (
			(settling_cycles_HB_LB_state.value()[LB_index]) |
			((settling_cycles_HB_LB_state.value()[HB_index] & 0b0000'0001) << (1 * 8))
		);
		return std::optional { settling_time_cycles };
	}

	std::optional<uint16_t> read_inc_num() {
		const size_t HB_index = 0;
		const size_t LB_index = 1;
		const std::optional<std::vector<uint8_t>> inc_num_HB_LB_state = block_read_register(RegisterAddrs::INC_NUM_HB, 2);
		if(inc_num_HB_LB_state.has_value() == false) {
			std::printf("AD5933: Failed to read the Frequency increment number:\n\tAD5933: Failed to read the INC_NUM_HB and INC_NUM_HB registers\n");
			return std::nullopt;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		const uint16_t inc_num = (
			(inc_num_HB_LB_state.value()[LB_index]) |
			((inc_num_HB_LB_state.value()[HB_index] & 0b0000'0001) << (1 * 8))
		);
		return std::optional<uint16_t> { inc_num };
	}

	std::optional<ControlLB::SYSCLK_SRC> read_sysclk_src() {
		const std::optional<uint8_t> control_LB_state = read_register(RegisterAddrs::CONTROL_LB);
		if(control_LB_state.has_value() == false) {
			std::printf("AD5933: Failed to read the SYSCLK_SRC by reading the CONTROL_LB register\n");
			return std::nullopt;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		const uint8_t sysclk_src_bit = control_LB_state.value() & 0b0000'1000;
		return std::optional<ControlLB::SYSCLK_SRC> { ControlLB::SYSCLK_SRC { sysclk_src_bit } };
	}

	std::optional<ControlHB::OutputExcitationVoltageRange> read_output_excitation_voltage_range() {
		const std::optional<uint8_t> control_HB_state = read_register(RegisterAddrs::CONTROL_HB);
		if(control_HB_state.has_value() == false) {
			std::printf("AD5933: Failed to read the Output Excitation Voltage Range by reading the CONTROL_HB register\n");
			return std::nullopt;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		const uint8_t output_excitation_voltage_range_bits = control_HB_state.value() & 0b0000'0110;
		return std::optional<ControlHB::OutputExcitationVoltageRange> {
			ControlHB::OutputExcitationVoltageRange { output_excitation_voltage_range_bits }
		};
	}

	std::optional<ControlHB::PGA_Gain> read_pga_gain() {
		const std::optional<uint8_t> control_HB_state = read_register(RegisterAddrs::CONTROL_HB);
		if(control_HB_state.has_value() == false) {
			std::printf("AD5933: Failed to read the PGA Gain by reading the CONTROL_HB register\n");
			return std::nullopt;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		const uint8_t pga_gain_bits = control_HB_state.value() & 0b0000'0001;
		return std::optional<ControlHB::PGA_Gain> { ControlHB::PGA_Gain { pga_gain_bits } };
	}	

	std::optional<uint32_t> read_frequency(const RegisterAddr_RW freq_register) {
		assert(
			(freq_register == RegisterAddrs::FREQ_START_HB) ||
			(freq_register == RegisterAddrs::FREQ_INC_HB)
		);

		const auto freq_register_HB_MB_LB_state = block_read_register(freq_register, 3);
		if(freq_register_HB_MB_LB_state.has_value() == false) {
			std::printf("AD5933: Failed to read frequency by reading: %s registers\n", freq_register == RegisterAddrs::FREQ_START_HB ? "FREQ_START_HB_MB_LB": "FREQ_INC_HB_MB_LB");
			return std::nullopt;
		}

		const auto frequency = convert_binary_coded_frequency_message_to_frequency( 
			std::array<uint8_t, 3> { 
				freq_register_HB_MB_LB_state.value()[0],
				freq_register_HB_MB_LB_state.value()[1],
				freq_register_HB_MB_LB_state.value()[2]
			}
		);

		if(frequency.has_value() == false) {
			std::printf("AD5933: Failed to read frequency: Failed to convert binary coded frequency message to frequency\n");
			return std::nullopt;
		}

		return frequency;
	}

	std::optional<std::array<int16_t, 2>> read_impedance_data() {
		const auto real_HB_LB_imag_HB_LB_state = block_read_register(AD5933::RegisterAddrs::REAL_DATA_HB, 4);
		if(real_HB_LB_imag_HB_LB_state.has_value() == false) {
			std::printf("AD5933: Failed to read the impedance data\n");
			return std::nullopt;
		}
		const int16_t real_value = (
			(static_cast<int16_t>(real_HB_LB_imag_HB_LB_state.value()[0]) << (1 * 8)) |
			(static_cast<int16_t>(real_HB_LB_imag_HB_LB_state.value()[1]) << (0 * 8))
		);

		const int16_t imag_value = (
			(static_cast<int16_t>(real_HB_LB_imag_HB_LB_state.value()[3]) << (1 * 8)) |
			(static_cast<int16_t>(real_HB_LB_imag_HB_LB_state.value()[4]) << (0 * 8))
		);
		return std::optional<std::array<int16_t, 2>> { std::array<int16_t, 2> { real_value, imag_value } };
	}

	inline double calculate_magnitude(const std::array<int16_t, 2> &impedance_data) {
		return sqrt(  static_cast<double>( ((impedance_data[0] * impedance_data[0]) + (impedance_data[1] * impedance_data[1])) )  );
	}

	inline double calculate_gain_factor(const double calibration_impedance, const double magnitude) {
		const double admittance = 1.0f / calibration_impedance;
		return admittance / magnitude;
	}

	inline double calculate_impedance(const double magnitude, const double gain_factor) {
		const double gain_factor_multiplied_by_magnitude = gain_factor * magnitude;
		const double result = 1.00f / gain_factor_multiplied_by_magnitude;
		return result;
	}

	inline double calculate_phase(const std::array<int16_t, 2> &impedance_data) {
		const double imag_divided_by_real = static_cast<double>(impedance_data[1]) / static_cast<double>(impedance_data[0]);
		return atanf(imag_divided_by_real);
	}
};

namespace AD5933_Tests {
	extern std::atomic<std::shared_ptr<AD5933>> ad5933;
	bool run_test_read_write_registers(AD5933 &ad5933);
	extern std::atomic<std::shared_ptr<AD5933>> ad5933;
	void init_ad5933(I2CBus &i2c_bus);
	bool manual_config();
	bool test_auto_config();
	bool impedance_try();
}
