#pragma once

#include <utility>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "ad5933/config/config.hpp"
#include "ad5933/calibration/calibration.hpp"

namespace ns {
    struct Calibration {
        float freq;
        float gain_factor;
        float sys_phase;
    };

    void to_json(json& j, const Calibration& p);
    void from_json(const json& j, Calibration& p);

    struct Config {
        AD5933::Masks::Or::Ctrl::HB::Command command;
        AD5933::Masks::Or::Ctrl::HB::VoltageRange range;
        AD5933::Masks::Or::Ctrl::HB::PGA_Gain pga_gain;
        AD5933::Masks::Or::Ctrl::LB::SysClkSrc sysclk_src;
        uint32_t start_freq;
        uint32_t inc_freq;
        uint16_t num_of_inc;
        uint16_t set_cycles_num;
        AD5933::Masks::Or::SettlingTimeCyclesHB::Multiplier set_cycles_mult;
    };

    void to_json(json& j, const Config& p);
    void from_json(const json& j, Config& p);

    struct CalibrationFile {
        Config config;
        std::vector<Calibration> calibration;
        CalibrationFile() = default;
        CalibrationFile(const AD5933::Config& in_config, const std::vector<AD5933::Calibration<float>>& in_calibration);
        std::pair<AD5933::Config, std::vector<AD5933::Calibration<float>>> to_data() const;
        json to_json() const;
    };

    void to_json(json& j, const CalibrationFile& p);
    void from_json(const json& j, CalibrationFile& p);
}