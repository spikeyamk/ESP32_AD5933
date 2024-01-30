#include <algorithm>
#include <cassert>

#include "json/conversion.hpp"

namespace ns {
    void to_json(json& j, const Calibration& p) {
        j = json{
            {"freq", p.freq},
            {"gain_factor", p.gain_factor},
            {"sys_phase", p.sys_phase}
        };
    }

    void from_json(const json& j, Calibration& p) {
        j.at("freq").get_to(p.freq);
        j.at("gain_factor").get_to(p.gain_factor);
        j.at("sys_phase").get_to(p.sys_phase);
    }

    void to_json(json& j, const Config& p) {
        j = json{
            {"command", p.command},
            {"range", p.range},
            {"pga_gain", p.pga_gain},
            {"sysclk_src", p.sysclk_src},
            {"start_freq", p.start_freq},
            {"inc_freq", p.inc_freq},
            {"num_of_inc", p.num_of_inc},
            {"set_cycles_num", p.set_cycles_num},
            {"set_cycles_mult", p.set_cycles_mult}
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

    void to_json(json& j, const CalibrationFile& p) {
        j = json{
            {"config", p.config},
            {"calibration", p.calibration},
        };
    }

    void from_json(const json& j, CalibrationFile& p) {
        j.at("config").get_to(p.config);
        j.at("calibration").get_to(p.calibration);
    }

    CalibrationFile::CalibrationFile(const AD5933::Config& in_config, const std::vector<AD5933::Calibration<float>>& in_calibration) {
        const auto start_freq = in_config.get_start_freq();
        const auto inc_freq = in_config.get_inc_freq();
        std::vector<float> frequency_vector(in_calibration.size());
        std::generate(
            frequency_vector.begin(),
            frequency_vector.end(),
            [start_freq, inc_freq, n = 0.0f] () mutable {
                return static_cast<float>(start_freq.unwrap() + ((n++) * inc_freq.unwrap()));
            }
        );
        std::vector<Calibration> tmp_calibration;
        for(size_t i = 0; i < frequency_vector.size(); i++) {
            tmp_calibration.push_back(
                Calibration {
                    frequency_vector[i],
                    in_calibration[i].get_gain_factor(),
                    in_calibration[i].get_system_phase(),
                }
            );
        }
        config = config;
        calibration = tmp_calibration;
    }

    std::pair<AD5933::Config, std::vector<AD5933::Calibration<float>>> CalibrationFile::to_data() const {
        AD5933::Config ret_config {
            config.command,
            config.range,
            config.pga_gain,
            config.sysclk_src,
            AD5933::uint_startfreq_t{ config.start_freq },
            AD5933::uint_incfreq_t{ config.inc_freq },
            config.num_of_inc,
            config.set_cycles_num,
            config.set_cycles_mult
        };

        std::vector<AD5933::Calibration<float>> ret_calibration;
        for(const auto& e: calibration) {
            ret_calibration.push_back(
                AD5933::Calibration{
                    e.gain_factor,
                    e.sys_phase
                }
            );
        }
        return { ret_config, ret_calibration };
    }

    json CalibrationFile::to_json() const {
        return json { *this };
    }
}