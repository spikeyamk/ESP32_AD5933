#include <array>
#include <cstdint>

#include "ad5933/config/config.hpp"
#include "ad5933/config/masks.hpp"

#include "ad5933/config/tests.hpp"

namespace AD5933 {
    namespace Config_Tests {
        constexpr AD5933::Config test_config {
            AD5933::Masks::Or::Ctrl::HB::Command::PowerDownMode,
            AD5933::Masks::Or::Ctrl::HB::VoltageRange::FourHundred_mVppk,
            AD5933::Masks::Or::Ctrl::HB::PGA_Gain::OneTime,

            AD5933::Masks::Or::Ctrl::LB::SysClkSrc::External,

            AD5933::uint_startfreq_t { 30'000 },
            AD5933::uint_incfreq_t { 10 },
            AD5933::uint9_t { 511 },

            AD5933::uint9_t { 511 },
            AD5933::Masks::Or::SettlingTimeCyclesHB::Multiplier::FourTimes
        };

        constexpr std::array<uint8_t, 12> test_config_array {
            0xA5,
            0x08,

            0x0F,
            0x5C,
            0x28,

            0x00,
            0x01,
            0x4F,

            0x01,
            0xFF,

            0x07,
            0xFF
        };

        constexpr AD5933::Config another_test_config {
            AD5933::Masks::Or::Ctrl::HB::Command::MeasureTemp,
            AD5933::Masks::Or::Ctrl::HB::VoltageRange::TwoHundred_mVppk,
            AD5933::Masks::Or::Ctrl::HB::PGA_Gain::OneTime,

            AD5933::Masks::Or::Ctrl::LB::SysClkSrc::Internal,

            AD5933::uint_startfreq_t { 50'000 },
            AD5933::uint_incfreq_t { 100 },
            AD5933::uint9_t { 123 },

            AD5933::uint9_t { 42 },
            AD5933::Masks::Or::SettlingTimeCyclesHB::Multiplier::TwoTimes
        };

        int test_getters() {
            using namespace AD5933::Masks::Or;
            if(test_config.get_command() != Ctrl::HB::Command::PowerDownMode) {
                return -1;
            } 

            if(test_config.get_voltage_range() != Ctrl::HB::VoltageRange::FourHundred_mVppk) {
                return -1;
            } 

            if(test_config.get_PGA_gain() != Ctrl::HB::PGA_Gain::OneTime) {
                return -1;
            } 

            if(test_config.get_sysclk_src() != Ctrl::LB::SysClkSrc::External) {
                return -1;
            } 

            if(test_config.get_start_freq() != uint_startfreq_t { 30'000 }) {
                return -1;
            } 

            if(test_config.get_inc_freq() != uint_incfreq_t { 10 }) {
                return -1;
            }

            if(test_config.get_num_of_inc() != AD5933::uint9_t { 511 }) {
                return -1;
            }
            
            if(test_config.get_settling_time_cycles_number() != AD5933::uint9_t { 511 }) {
                return -1;
            }

            if(test_config.get_settling_time_cycles_multiplier() != SettlingTimeCyclesHB::Multiplier::FourTimes) {
                return -1;
            }

            return 0;
        }

        int test_setters(const AD5933::Config &in_config) {
            auto config_to_check_against = test_config;
            using namespace AD5933::Masks::Or;
            config_to_check_against.set_command(in_config.get_command());
            if(config_to_check_against.get_command() != in_config.get_command()) {
                return -1;
            } 

            config_to_check_against.set_voltage_range(in_config.get_voltage_range());
            if(config_to_check_against.get_voltage_range() != in_config.get_voltage_range()) {
                return -2;
            } 

            config_to_check_against.set_PGA_gain(in_config.get_PGA_gain());
            if(config_to_check_against.get_PGA_gain() != in_config.get_PGA_gain()) {
                return -3;
            } 

            config_to_check_against.set_sysclk_src(in_config.get_sysclk_src());
            if(config_to_check_against.get_sysclk_src() != in_config.get_sysclk_src()) {
                return -4;
            } 

            config_to_check_against.set_start_freq(in_config.get_start_freq());
            if(in_config.get_start_freq() != in_config.get_start_freq()) {
                return -5;
            } 

            config_to_check_against.set_inc_freq(in_config.get_inc_freq());
            if(in_config.get_inc_freq() != in_config.get_inc_freq()) {
                return -6;
            }

            config_to_check_against.set_num_of_inc(in_config.get_num_of_inc());
            if(in_config.get_num_of_inc() != in_config.get_num_of_inc()) {
                return -7;
            }
            
            config_to_check_against.set_settling_time_cycles_number(in_config.get_settling_time_cycles_number());
            if(in_config.get_settling_time_cycles_number() != in_config.get_settling_time_cycles_number()) {
                return -8;
            }

            config_to_check_against.set_settling_time_cycles_multiplier(in_config.get_settling_time_cycles_multiplier());
            if(in_config.get_settling_time_cycles_multiplier() != in_config.get_settling_time_cycles_multiplier()) {
                return -9;
            }

            return 0;
        }

        int test_same_setters() {
            return test_setters(test_config);
        }

        int test_different_setters() {
            return test_setters(another_test_config);
        }

        int test_to_array_to_config() {
            if(test_config.to_raw_array() == another_test_config.to_raw_array()) {
                return -1;
            }

            if(test_config.to_bitset_array() == another_test_config.to_bitset_array()) {
                return -1;
            }

            const AD5933::Config config_to_check_against { test_config.to_raw_array() };
            if(config_to_check_against.to_raw_array() != test_config.to_raw_array()) {
                return -1;
            }

            if(config_to_check_against.to_bitset_array() != test_config.to_bitset_array()) {
                return -1;
            }

            return 0;
        }

        int test_array() {
            if(test_config_array != test_config.to_raw_array()) {
                return -1;
            }

            return 0;
        }
    }
}
