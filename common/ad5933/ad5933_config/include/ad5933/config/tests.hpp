#pragma once

#include "ad5933/config/config.hpp"
#include "ad5933/masks/masks.hpp"

namespace AD5933 {
    namespace Config_Tests {
        int test_getters();
        int test_setters(const AD5933::Config &in_config);
        int test_same_setters();
        int test_different_setters();
        int test_to_array_to_config();
        int test_array();

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
    }
}