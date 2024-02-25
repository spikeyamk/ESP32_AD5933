#pragma once

#include <utility>
#include <cstdint>
#include <vector>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "ad5933/config/config.hpp"
#include "ad5933/calibration/calibration.hpp"

namespace ns {
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
        Config() = default;
        Config(const AD5933::Config& config);
    };

    struct Point {
        float gain_factor;
        float sys_phase;
        Point() = default;
        Point(const AD5933::Calibration<float>& ad5933_calibration);
    };

    struct Calibration {
        float impedance;
        Config config;
        std::vector<Point> points;
        Calibration() = default;
        Calibration(const float impedance, const AD5933::Config& config, const std::vector<AD5933::Calibration<float>>& calibration_vector);
    };

    struct CalibrationFile {
        Calibration calibration;
        CalibrationFile() = default;
        CalibrationFile(const float impedance, const AD5933::Config& config, const std::vector<AD5933::Calibration<float>>& calibration_vector);
        std::pair<AD5933::Config, std::vector<AD5933::Calibration<float>>> unwrap() const;
    };
}

namespace ns {
    void to_json(json& j, const Config& p);
    void from_json(const json& j, Config& p);
    void to_json(json& j, const Point& p);
    void from_json(const json& j, Point& p);
    void to_json(json& j, const Calibration& p);
    void from_json(const json& j, Calibration& p);
    void to_json(json& j, const CalibrationFile& p);
    void from_json(const json& j, CalibrationFile& p);
}