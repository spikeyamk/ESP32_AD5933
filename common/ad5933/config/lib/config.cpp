
#include <iostream>

#include "ad5933/config/maps.hpp"

#include "ad5933/config/config.hpp"

namespace AD5933 {
    void Config::print() const {
        std::printf("AD5933::Config::print(): this: '%p'\n", reinterpret_cast<const void*>(this));
        std::cout << "\tCommand: " << get_command() << std::endl;
        std::cout << "\tRange: " << get_voltage_range() << std::endl;
        std::cout << "\tPGA Gain: " << get_PGA_gain() << std::endl;
        std::cout << "\tSystem clock: " << get_sysclk_src() << std::endl;
        std::cout << "\tStart frequency: " << get_start_freq() << std::endl;
        std::cout << "\tIncrement frequency: " << get_inc_freq() << std::endl;
        std::cout << "\tNumber of increments: " << get_num_of_inc() << std::endl;
        std::cout << "\tSettling cycles number: " << get_settling_time_cycles_number() << std::endl;
        std::cout << "\tSettling cycles multiplier: " << get_settling_time_cycles_multiplier() << std::endl;
    }
}