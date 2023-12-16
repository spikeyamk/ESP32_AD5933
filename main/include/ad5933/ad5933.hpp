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
#include <type_traits>
#include <stdexcept>
#include <span>

#include "trielo/trielo.hpp"

#include "util.hpp"

class AD5933 : public I2CDevice {
	std::optional<float> measure_temperature() {
		if(set_control_command(ControlHB::Commands::MEASURE_TEMPERATURE) == false) {
			std::printf("AD5933: Measure Temperature failed: Failed to write the MEASURE_TEMPERATURE command to CONTROL_HB register\n");
			return std::nullopt;
		}
		while(has_status_condition(Status::TEMP_VALID) == false) {
			std::printf("AD5933: Measure Temperature: Waiting for TEMP_VALID in the STATUS register\n");
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}

		const auto temp_data { block_read_register<2>(RegisterAddrs::RW_RO::TEMP_DATA_HB) };
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
		const std::optional<uint8_t> status_state = read_register(RegisterAddrs::RW_RO::STATUS);
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
};

namespace AD5933_Tests {
	extern std::atomic<std::shared_ptr<AD5933>> ad5933;
	bool run_test_read_write_registers(AD5933 &ad5933);
	extern std::atomic<std::shared_ptr<AD5933>> ad5933;
	void init_ad5933(Bus &i2c_bus);
	bool manual_config();
	bool test_auto_config();
	bool impedance_try();
}
