#include "json/conversion.hpp"

namespace ns {
    Config::Config(const AD5933::Config& config) :
        command { config.get_command() },
        range { config.get_voltage_range() },
        pga_gain { config.get_PGA_gain() },
        sysclk_src { config.get_sysclk_src() },
        start_freq { config.get_start_freq().unwrap() },
        inc_freq { config.get_inc_freq().unwrap() },
        num_of_inc { config.get_num_of_inc().unwrap() },
        set_cycles_num { config.get_settling_time_cycles_number().unwrap() },
        set_cycles_mult { config.get_settling_time_cycles_multiplier() }
    {}

    Point::Point(const AD5933::Calibration<float>& ad5933_calibration) :
        gain_factor { ad5933_calibration.get_gain_factor() },
        sys_phase { ad5933_calibration.get_system_phase() }
    {}

    Calibration::Calibration(const float impedance, const AD5933::Config& config, const std::vector<AD5933::Calibration<float>>& calibration_vector) :
        impedance { impedance },
        config { config },
        points {
            [&]() {
                std::vector<Point> tmp_points;
                tmp_points.reserve(calibration_vector.size());
                for(const AD5933::Calibration<float>& e: calibration_vector) {
                    tmp_points.push_back(Point { e });
                }
                return tmp_points;
            }()
        }
    {}

    CalibrationFile::CalibrationFile(const float impedance, const AD5933::Config& config, const std::vector<AD5933::Calibration<float>>& calibration_vector) :
        calibration { impedance, config, calibration_vector }
    {}

    std::pair<AD5933::Config, std::vector<AD5933::Calibration<float>>> CalibrationFile::unwrap() const {
        const AD5933::Config ret_config { 
            calibration.config.command,
            calibration.config.range,
            calibration.config.pga_gain,
            calibration.config.sysclk_src,
            AD5933::uint_startfreq_t { calibration.config.start_freq },
            AD5933::uint_incfreq_t { calibration.config.inc_freq },
            AD5933::uint9_t { calibration.config.num_of_inc },
            AD5933::uint9_t { calibration.config.set_cycles_num },
            calibration.config.set_cycles_mult
        };
        std::vector<AD5933::Calibration<float>> ret_calibration; 
        for(const Point& e: calibration.points) {
            ret_calibration.push_back(AD5933::Calibration<float>{ e.gain_factor, e.sys_phase });
        }
        return { ret_config, ret_calibration };
    }
}

namespace ns {
    void to_json(json& j, const Config& p) {
        j = json {
            { "command", p.command },
            { "range", p.range },
            { "pga_gain", p.pga_gain },
            { "sysclk_src", p.sysclk_src },
            { "start_freq", p.start_freq },
            { "inc_freq", p.inc_freq },
            { "num_of_inc", p.num_of_inc },
            { "set_cycles_num", p.set_cycles_num },
            { "set_cycles_mult", p.set_cycles_mult },
        };
    }

    void from_json(const json& j, Config& p) {
        j.at("command").get_to(p.command);
        j.at("range").get_to(p.range);
        j.at("pga_gain").get_to(p.pga_gain);
        j.at("sysclk_src").get_to(p.sysclk_src);
        j.at("start_freq").get_to(p.start_freq);
        j.at("inc_freq").get_to(p.inc_freq);
        j.at("num_of_inc").get_to(p.num_of_inc);
        j.at("set_cycles_num").get_to(p.set_cycles_num);
        j.at("set_cycles_mult").get_to(p.set_cycles_mult);
    }
}

namespace ns {
    void to_json(json& j, const Point& p) {
        j = json {
            { "gain_factor", p.gain_factor },
            { "sys_phase", p.sys_phase },
        };
    }
    
    void from_json(const json& j, Point& p) {
        j.at("gain_factor").get_to(p.gain_factor);
        j.at("sys_phase").get_to(p.sys_phase);
    }
}

namespace ns {
    void to_json(json& j, const Calibration& p) {
        j = json {
            { "impedance", p.impedance },
            { "config", p.config },
            { "points", p.points },
        };
    }
    
    void from_json(const json& j, Calibration& p) {
        j.at("impedance").get_to(p.impedance);
        j.at("config").get_to(p.config);
        j.at("points").get_to(p.points);
    }
}

namespace ns {
    void to_json(json& j, const CalibrationFile& p) {
        j = json {
            { "calibration", p.calibration },
        };
    }

    void from_json(const json& j, CalibrationFile& p) {
        j.at("calibration").get_to(p.calibration);
    }
}
