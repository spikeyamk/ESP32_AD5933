#pragma once

#include <cmath>

#include "ad5933/data/data.hpp"
#include "ad5933/calibration/calibration.hpp"

namespace AD5933 {
    template<typename T_Floating>
    class Measurement {
    private:
        T_Floating magnitude;
        T_Floating phase;
    public:
        Measurement() = delete;

        constexpr Measurement(const Data &raw_data, const Calibration<T_Floating> &calibration) :
            magnitude { calibration.get_gain_factor() / raw_data.get_raw_magnitude<T_Floating>() },
            phase { raw_data.get_raw_phase<T_Floating>() - calibration.get_system_phase() }
        {}

        constexpr T_Floating get_magnitude() const {
            return magnitude;
        }

        constexpr T_Floating get_phase() const {
            return phase;
        }

        constexpr T_Floating get_resistance() const {
            return magnitude * std::cos(phase);
        }

        constexpr T_Floating get_reactance() const {
            return magnitude * std::sin(phase);
        }
    };
}