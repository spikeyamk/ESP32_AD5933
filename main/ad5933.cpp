#include <iostream>
#include <thread>
#include <chrono>
#include <cassert>

#include "ad5933.hpp"
#include "i2c.hpp"

namespace AD5933_Tests {
	std::atomic<std::shared_ptr<AD5933>> ad5933 = nullptr;

	void init_ad5933(I2CBus &i2c_bus) {
		while(i2c_bus.device_add(AD5933::SLAVE_ADDRESS) == false) {
			std::printf("I2CBus: Failed to add AD5933\n");
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}

		ad5933 = std::make_shared<AD5933>(i2c_bus.active_device_handles[AD5933::SLAVE_ADDRESS]);
	}
}

std::ostream& operator<<(std::ostream& os, const AD5933::SettlingCyclesHB::Multiplier m) {
	switch(m) {
		case AD5933::SettlingCyclesHB::Multiplier::ONE_TIME:   return os << "ONE_TIME (" << std::bitset<2>(static_cast<int>(m)) << ")";
		case AD5933::SettlingCyclesHB::Multiplier::TWO_TIMES:  return os << "TWO_TIMES (" << std::bitset<2>(static_cast<int>(m)) << ")";
		case AD5933::SettlingCyclesHB::Multiplier::FOUR_TIMES: return os << "FOUR_TIMES (" << std::bitset<2>(static_cast<int>(m)) << ")";
		default: return os << "Invalid multiplier";
	}
}

std::ostream& operator<<(std::ostream& os, AD5933::ControlLB::SYSCLK_SRC src) {
    switch (src) {
        case AD5933::ControlLB::SYSCLK_SRC::INT_SYSCLK:
            os << "INT_SYSCLK (" << std::bitset<3>(static_cast<int>(src)) << ")";
            break;
        case AD5933::ControlLB::SYSCLK_SRC::EXT_SYSCLK:
            os << "EXT_SYSCLK (" << std::bitset<3>(static_cast<int>(src)) << ")";
            break;
        default:
            os << "Unknown";
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, AD5933::ControlHB::OutputExcitationVoltageRange range) {
    switch (range) {
        case AD5933::ControlHB::OutputExcitationVoltageRange::TWO_VOLT_PPK:
            os << "TWO_VOLT_PPK (" << std::bitset<2>(static_cast<int>(range)) << ")";
            break;
        case AD5933::ControlHB::OutputExcitationVoltageRange::ONE_VOLT_PPK:
            os << "ONE_VOLT_PPK (" << std::bitset<2>(static_cast<int>(range)) << ")";
            break;
        case AD5933::ControlHB::OutputExcitationVoltageRange::FOUR_HUNDRED_MILI_VOLT_PPK:
            os << "FOUR_HUNDRED_MILI_VOLT_PPK (" << std::bitset<2>(static_cast<int>(range)) << ")";
            break;
        case AD5933::ControlHB::OutputExcitationVoltageRange::TWO_HUNDRED_MILI_VOLT_PPK:
            os << "TWO_HUNDRED_MILI_VOLT_PPK (" << std::bitset<2>(static_cast<int>(range)) << ")";
            break;
        default:
            os << "Unknown";
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, AD5933::ControlHB::PGA_Gain gain) {
    switch (gain) {
        case AD5933::ControlHB::PGA_Gain::FiveTimes:
            os << "FiveTimes";
            break;
        case AD5933::ControlHB::PGA_Gain::OneTime:
            os << "OneTime";
            break;
        default:
            os << "Unknown";
    }
    return os;
}
