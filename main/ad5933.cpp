#include <thread>
#include <chrono>

#include "ad5933.hpp"
#include "i2c.hpp"

namespace AD5933_Tests {
	bool test_CONTROL_HB(AD5933 &ad5933) {
		if(ad5933.set_pga_gain(AD5933::ControlHB::PGA_Gain::FiveTimes) == false) {
			std::printf("AD5933: Failed to set the PGA_Gain to: FiveTimes\n");
			return false;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		if(ad5933.set_pga_gain(AD5933::ControlHB::PGA_Gain::OneTime) == false) {
			std::printf("AD5933: Failed to set the PGA_Gain to: OneTime\n");
			return false;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		if(ad5933.set_control_command(AD5933::ControlHB::Commands::MEASURE_TEMPERATURE) == false) {
			std::printf("AD5933: Failed to set the ControlHB::Command to: MEASURE_TEMPERATURE\n");
			return false;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		if(ad5933.set_control_command(AD5933::ControlHB::Commands::STANDBY_MODE) == false) {
			std::printf("AD5933: Failed to set the ControlHB::Command to: STANDBY_MODE\n");
			return false;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		if(ad5933.set_control_command(AD5933::ControlHB::Commands::POWER_DOWN_MODE) == false) {
			std::printf("AD5933: Failed to set the ControlHB::Command to: POWER_DOWN_MODE\n");
			return false;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		if(ad5933.set_output_excitation_voltage_range(AD5933::ControlHB::OutputExcitationVoltageRange::FOUR_HUNDRED_MILI_VOLT_PPK) == false) {
			std::printf("AD5933: Failed to set the Output Excitation Voltage to: FOUR_HUNDRED_MILI_VOLT_PPK\n");
			return false;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		if(ad5933.set_output_excitation_voltage_range(AD5933::ControlHB::OutputExcitationVoltageRange::TWO_HUNDRED_MILI_VOLT_PPK) == false) {
			std::printf("AD5933: Failed to set the Output Excitation Voltage to: TWO_HUNDRED_MILI_VOLT_PPK\n");
			return false;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		if(ad5933.set_output_excitation_voltage_range(AD5933::ControlHB::OutputExcitationVoltageRange::ONE_VOLT_PPK) == false) {
			std::printf("AD5933: Failed to set the Output Excitation Voltage to: ONE_VOLT_PPK\n");
			return false;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		if(ad5933.set_output_excitation_voltage_range(AD5933::ControlHB::OutputExcitationVoltageRange::TWO_VOLT_PPK) == false) {
			std::printf("AD5933: Failed to set the Output Excitation Voltage to: TWO_VOLT_PPK\n");
			return false;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		if(ad5933.set_output_excitation_voltage_range(AD5933::ControlHB::OutputExcitationVoltageRange::ONE_VOLT_PPK) == false) {
			std::printf("AD5933: Failed to set the Output Excitation Voltage to: ONE_VOLT_PPK\n");
			return false;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		if(ad5933.set_range_and_gain(AD5933::ControlHB::OutputExcitationVoltageRange::FOUR_HUNDRED_MILI_VOLT_PPK, 
			AD5933::ControlHB::PGA_Gain::FiveTimes) == false) {
			std::printf("AD5933: Failed to set the Output Excitation Voltage to: FOUR_HUNDRED_MILI_VOLT_PPK and PGA Gain to: FiveTimes\n");
			return false;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		return true;
	}

	bool test_CONTROL_LB(AD5933 &ad5933) {
		if(ad5933.reset() == false) {
			std::printf("AD5933: Reset failed\n");
			return false;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		// Setting clk source to internal
		if(ad5933.set_sysclk(AD5933::SYSCLK_SRC::INT_SYSCLK) == false) {
			std::printf("AD5933: Failed to select INT_SYSCLK\n");
			return false;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		if(ad5933.reset() == false) {
			std::printf("AD5933: Reset failed\n");
			return false;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		// Setting clk source to external
		if(ad5933.set_sysclk(AD5933::SYSCLK_SRC::EXT_SYSCLK) == false) {
			std::printf("AD5933: Failed to select EXT_SYSCLK\n");
			return false;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		if(ad5933.reset() == false) {
			std::printf("AD5933: Reset failed\n");
			return false;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		// Setting clk source to external
		if(ad5933.set_sysclk(AD5933::SYSCLK_SRC::EXT_SYSCLK) == false) {
			std::printf("AD5933: Failed to select EXT_SYSCLK\n");
			return false;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		if(ad5933.reset() == false) {
			std::printf("AD5933: Reset failed\n");
			return false;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		// Setting clk source to internal
		if(ad5933.set_sysclk(AD5933::SYSCLK_SRC::INT_SYSCLK) == false) {
			std::printf("AD5933: Failed to select INT_SYSCLK\n");
			return false;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		if(ad5933.reset() == false) {
			std::printf("AD5933: Reset failed\n");
			return false;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		return true;
	}

	bool test_RESET(AD5933 &ad5933) {
		if(ad5933.reset() == false) {
			std::printf("AD5933: Reset failed\n");
			return false;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		return true;
	}

	bool test_START_FREQ_HB_MB_LB(AD5933 &ad5933) {
		//for(uint32_t start_freq_to_set = 30'000; start_freq_to_set <= 0xFFFFFF; start_freq_to_set++) {
		for(uint32_t start_freq_to_set = 30'000; start_freq_to_set <= 30'010; start_freq_to_set++) {
			if(ad5933.set_start_freq(start_freq_to_set) == false) {
				std::printf("AD5933: Failed to set the start frequency to: %lu\n", start_freq_to_set);
				return false;
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
		return true;
	}

	bool test_INC_FREQ_HB_MB_LB(AD5933 &ad5933) {
		//for(uint32_t inc_freq_to_set = 10; inc_freq_to_set <= 0xFFFFFF; inc_freq_to_set++) {
		for(uint32_t inc_freq_to_set = 90; inc_freq_to_set <= 100; inc_freq_to_set++) {
			if(ad5933.set_inc_freq(inc_freq_to_set) == false) {
				std::printf("AD5933: Failed to set the increment frequency to: %lu\n", inc_freq_to_set);
				return false;
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
		return true;
	}

	bool test_INC_NUM_HB_LB(AD5933 &ad5933) {
		//for(uint16_t inc_num_to_set = 1; inc_num_to_set <= 0x1FF; inc_num_to_set++) {
		for(uint16_t inc_num_to_set = 2; inc_num_to_set <= 0x10; inc_num_to_set++) {
			if(ad5933.set_inc_num(inc_num_to_set) == false) {
				std::printf("AD5933: Failed to set the increment number to: %u\n", inc_num_to_set);
				return false;
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
		return true;
	}

	bool test_FREQ_SWEEP_CONFIG(AD5933 &ad5933) {
		for(uint16_t start_freq = 30'000, inc_freq = 10, inc_num = 100; start_freq <= 30'020; start_freq++, inc_freq++, inc_num++) {
			if(ad5933.configure_sweep(start_freq, inc_freq, inc_num) == false) {
				std::printf("AD5933: Failed to configure sweep to: start_freq: %u, inc_freq: %u, inc_num: %u\n", start_freq, inc_freq, inc_num);
				return false;
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
		return true;
	}

	bool test_DEFAULT_FREQ_SWEEP_CONFIG(AD5933 &ad5933) {
		if(ad5933.set_sysclk(AD5933::SYSCLK_SRC::EXT_SYSCLK) == false) {
			std::printf("AD5933: Failed to set the SYSCLK_SRC to EXT_SYSCLK\n");
			return false;
		}

		uint16_t start_freq = 30'000, inc_freq = 10, inc_num = 100;
		if(ad5933.configure_sweep(start_freq, inc_freq, inc_num) == false) {
			std::printf("AD5933: Failed to configure sweep to: start_freq: %u, inc_freq: %u, inc_num: %u\n", start_freq, inc_freq, inc_num);
			return false;
		}

		const uint8_t THIRTHY_KILOHERTZ_FREQ_START_HB = 0x0F;
		const uint8_t THIRTHY_KILOHERTZ_FREQ_START_MB = 0x5C;
		const uint8_t THIRTHY_KILOHERTZ_FREQ_START_LB = 0x28;

		const uint8_t TEN_HERTZ_FREQ_INC_HB = 0x00;
		const uint8_t TEN_HERTZ_FREQ_INC_MB = 0x01;
		const uint8_t TEN_HERTZ_FREQ_INC_LB = 0x4F;


		const uint8_t FREQ_START_HB_state = ad5933.read_register(AD5933::RegisterAddrs::FREQ_START_HB).value_or(0xFF);
		const uint8_t FREQ_START_MB_state = ad5933.read_register(AD5933::RegisterAddrs::FREQ_START_MB).value_or(0xFF);
		const uint8_t FREQ_START_LB_state = ad5933.read_register(AD5933::RegisterAddrs::FREQ_START_LB).value_or(0xFF);
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		const uint8_t FREQ_INC_HB_state = ad5933.read_register(AD5933::RegisterAddrs::FREQ_INC_HB).value_or(0xFF);
		const uint8_t FREQ_INC_MB_state = ad5933.read_register(AD5933::RegisterAddrs::FREQ_INC_MB).value_or(0xFF);
		const uint8_t FREQ_INC_LB_state = ad5933.read_register(AD5933::RegisterAddrs::FREQ_INC_LB).value_or(0xFF);
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		if(!(
			FREQ_START_HB_state == THIRTHY_KILOHERTZ_FREQ_START_HB &&
			FREQ_START_MB_state == THIRTHY_KILOHERTZ_FREQ_START_MB &&
			FREQ_START_LB_state == THIRTHY_KILOHERTZ_FREQ_START_LB
		)) {
			std::printf("AD5933: Failed to verify the state of FREQ_START registers against default 30'000 Hz value\n");
			std::printf("\tDefault 30'000 Hz value:\n"
							"THIRTHY_KILOHERTZ_FREQ_START_HB: 0x%02X\n"
							"THIRTHY_KILOHERTZ_FREQ_START_MB: 0x%02X\n"
							"THIRTHY_KILOHERTZ_FREQ_START_LB: 0x%02X\n"
						
						
							"FREQ_START_HB_state: 0x%02X\n"
							"FREQ_START_MB_state: 0x%02X\n"
							"FREQ_START_LB_state: 0x%02X\n"
						,
						THIRTHY_KILOHERTZ_FREQ_START_HB,
						THIRTHY_KILOHERTZ_FREQ_START_MB,
						THIRTHY_KILOHERTZ_FREQ_START_LB,

						FREQ_START_HB_state,
						FREQ_START_MB_state,
						FREQ_START_LB_state
			);
			return false;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		if(!(
			FREQ_INC_HB_state == TEN_HERTZ_FREQ_INC_HB &&
			FREQ_INC_MB_state == TEN_HERTZ_FREQ_INC_MB &&
			FREQ_INC_LB_state == TEN_HERTZ_FREQ_INC_LB
		)) {
			std::printf("AD5933: Failed to verify the state of FREQ_INC registers against default 10 Hz value\n");
			return false;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		return true;
	}

	bool test_SETTLING_TIME_OPTIONS(AD5933 &ad5933) {
		if(ad5933.set_settling_time_multiplier(AD5933::SettlingCyclesHB::Multiplier::FOUR_TIMES) == false) {
			std::printf("AD5933: Failed to set the settling time cycles multiplier to: FOUR_TIMES\n");
			return false;
		}

		if(ad5933.set_settling_time_cycles(15) == false) {
			std::printf("AD5933: Failed to set the settling time cycles number to: 15\n");
			return false;
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		return true;
	}

	bool run_test_read_write_registers(AD5933 &ad5933) {
		std::map<const char*, std::function<bool(AD5933&)>> array_of_functions {
			{"test_CONTROL_HB", test_CONTROL_HB},
			{"test_CONTROL_LB", test_CONTROL_LB},
			{"test_RESET", test_RESET},
			{"test_START_FREQ_HB_MB_LB", test_START_FREQ_HB_MB_LB},
			{"test_INC_FREQ_HB_MB_LB", test_INC_FREQ_HB_MB_LB},
			{"test_INC_NUM_HB_LB", test_INC_NUM_HB_LB},
			{"test_FREQ_SWEEP_CONFIG", test_FREQ_SWEEP_CONFIG},
			{"test_DEFAULT_FREQ_SWEEP_CONFIG", test_DEFAULT_FREQ_SWEEP_CONFIG},
			{"test_SETTLING_TIME_OPTIONS", test_SETTLING_TIME_OPTIONS}
		};

		std::printf("AD5933: Testing Read/Write registers: done: 0/%u\n", array_of_functions.size());
		size_t i = 1;
		for(const auto& [func_name, func_ptr] : array_of_functions) {
			if(func_ptr(ad5933) == false) {
				std::printf("AD5933: ERROR: Testing Read/Write registers: done: %u/%u\n", i, array_of_functions.size());
				std::printf("AD5933: %s failed. Exiting...\n", func_name);
				return false;
			}
			std::printf("AD5933: Testing Read/Write registers: done: %u/%u\n", i, array_of_functions.size());
			std::printf("AD5933: Success: %s\n", func_name);
			i++;
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		return true;
	}

	bool impedance_sweep(AD5933 &ad5933) {
		ad5933.set_sysclk(AD5933::SYSCLK_SRC::EXT_SYSCLK);
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		ad5933.set_pga_gain(AD5933::ControlHB::PGA_Gain::OneTime);
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		ad5933.set_output_excitation_voltage_range(AD5933::ControlHB::OutputExcitationVoltageRange::TWO_VOLT_PPK);
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		ad5933.set_settling_time_cycles(15);
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		ad5933.set_settling_time_multiplier(AD5933::SettlingCyclesHB::Multiplier::ONE_TIME);
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		if(ad5933.configure_sweep(30'000, 10, 10) == false) {
			std::printf("AD5933: Impedance Sweep failed: Failed to configure sweep\n");
			return false;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		ad5933.dump_all_registers();
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		if(ad5933.reset() == false) {
			std::printf("AD5933: Impedance Sweep failed: Failed to reset\n");
			return false;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		/*
		if(ad5933.set_control_command(
			AD5933::ControlHB::Commands::INITIALIZE_WITH_START_FREQUENCY
		) == false) {
			std::printf("AD5933: Impedance Sweep failed: Failed to set the INITIALIZE_WITH_START_FREQUENCY CONTROL_HB command\n");
			return false;
		}

		if(ad5933.set_control_command(
			AD5933::ControlHB::Commands::START_FREQUENCY_SWEEP
		) == false) {
			std::printf("AD5933: Impedance Sweep failed: Failed to set the START_FREQUENCY_SWEEP CONTROL_HB command\n");
			return false;
		}
		*/

		ad5933.set_control_command(AD5933::ControlHB::Commands::INITIALIZE_WITH_START_FREQUENCY);
		ad5933.set_control_command(AD5933::ControlHB::Commands::START_FREQUENCY_SWEEP);

		std::vector<std::vector<uint8_t>> accumulated_impedance_data;
		do {
			while(ad5933.has_status_condition(AD5933::Status::IMPEDANCE_VALID) == false) {
				std::printf("AD5933: Status: IMPEDANCE_VALID is not ready\n");
				std::printf("AD5933: Status: 0x%02x\n", ad5933.read_register(AD5933::RegisterAddrs::STATUS).value_or(0xFF));
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
			}

			const std::optional<std::vector<uint8_t>> impedance_data = ad5933.block_read_register(
				AD5933::RegisterAddrs::REAL_DATA_HB,
				4
			);
			std::this_thread::sleep_for(std::chrono::milliseconds(100));

			if(impedance_data.has_value() == false) {
				return false;
			}
			accumulated_impedance_data.push_back(impedance_data.value());
			std::this_thread::sleep_for(std::chrono::milliseconds(100));

			if(ad5933.set_control_command(AD5933::ControlHB::Commands::INCREMENT_FREQUENCY) == false) {
				std::printf("AD5933: Impedance Sweep failed: Failed to set the INCREMENT_FREQUENCY CONTROL_HB command\n");
			}
		} while(ad5933.has_status_condition(AD5933::Status::SWEEP_DONE) != false);
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		size_t i = 0;
		for(const auto &measured_impedance: accumulated_impedance_data) {
			const int16_t real = *(reinterpret_cast<const int16_t*>(measured_impedance.data()));
			const int16_t imag = *(reinterpret_cast<const int16_t*>(measured_impedance.data() + 2));
			std::printf("measured_impedance[%u]: real: %d imag: %d\n", i, real, imag);
			i++;
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
		return true;
	}

	bool arduino_emulated_impedance_sweep(AD5933 &ad5933) {
		ad5933.set_control_command(AD5933::ControlHB::Commands::STANDBY_MODE);
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		ad5933.set_control_command(AD5933::ControlHB::Commands::INITIALIZE_WITH_START_FREQUENCY);
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		ad5933.set_control_command(AD5933::ControlHB::Commands::START_FREQUENCY_SWEEP);
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		while(
			(ad5933.read_register(AD5933::RegisterAddrs::STATUS).value_or(0x00) &
				static_cast<uint8_t>(AD5933::Status::SWEEP_DONE)
			) !=  static_cast<uint8_t>(AD5933::Status::SWEEP_DONE)
		) {
			const auto data = ad5933.block_read_register(AD5933::RegisterAddrs::REAL_DATA_HB, 4).value();
			std::printf("real: 0x%02X%02X imag: 0x%02X%02X\n", 
				data[0], data[1],
				data[2], data[3]
			);
			while(ad5933.has_status_condition(AD5933::Status::IMPEDANCE_VALID) == false) {
				std::printf("bohuzial\n");
				ad5933.dump_all_registers();
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
			}
			ad5933.dump_all_registers();
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			ad5933.set_control_command(AD5933::ControlHB::Commands::INCREMENT_FREQUENCY);
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
		return ad5933.set_control_command(AD5933::ControlHB::Commands::STANDBY_MODE);
	}

	std::atomic<std::shared_ptr<AD5933>> ad5933 = nullptr;

	void init_ad5933(I2CBus &i2c_bus) {
		while(i2c_bus.device_add(AD5933::SLAVE_ADDRESS) == false) {
			std::printf("I2CBus: Failed to add AD5933\n");
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}

		ad5933 = std::make_shared<AD5933>(i2c_bus.active_device_handles[AD5933::SLAVE_ADDRESS]);
		ad5933.load()->dump_all_registers();
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
}
