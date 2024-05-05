#pragma once

#include "ad5933/data/data.hpp"

namespace AD5933 {
    template<typename T_Floating>
    class Calibration {
    private:
        T_Floating gain_factor;
        T_Floating system_phase;
    public:
        Calibration() = delete;

        constexpr inline Calibration(const T_Floating raw_magnitude, const T_Floating raw_phase, const T_Floating calibration_impedance) :
            gain_factor {init_gain_factor(raw_magnitude, calibration_impedance)},
            system_phase {raw_phase}
        {}

        constexpr inline Calibration(Data raw_data, const T_Floating calibration_impedance) :
            gain_factor {init_gain_factor(raw_data.get_raw_magnitude<T_Floating>(), calibration_impedance)},
            system_phase {raw_data.get_raw_phase<T_Floating>()}
        {}

        constexpr inline Calibration(T_Floating gain_factor, T_Floating system_phase) :
            gain_factor { gain_factor },
            system_phase { system_phase }
        {}
    public:
        constexpr inline T_Floating get_gain_factor() const {
            return gain_factor;
        }

        constexpr inline T_Floating get_system_phase() const {
            return system_phase;
        }
    private:
        constexpr inline T_Floating init_gain_factor(const T_Floating raw_magnitude, const T_Floating calibration_impedance) {
            return raw_magnitude * calibration_impedance;
        }
    };
}