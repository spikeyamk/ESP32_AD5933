#include "ad5933/calibration/tests.hpp"

namespace AD5933 {
    namespace Calibration_Tests {
        template<typename T_Floating>
        int test_constructor_templated() {
            constexpr T_Floating raw_magnitude = static_cast<T_Floating>(20'000.0);
            constexpr T_Floating raw_phase = static_cast<T_Floating>(1.5);
            constexpr T_Floating calibration_impedance = static_cast<T_Floating>(10'000.0);
            const AD5933::Calibration<T_Floating> test_calibration { raw_magnitude, raw_phase, calibration_impedance };

            if(test_calibration.get_system_phase() != raw_phase) {
                return -1;
            }

            constexpr T_Floating gain_factor = raw_magnitude * calibration_impedance ;
            if(test_calibration.get_gain_factor() != gain_factor) {
                return -1;
            }

            return 0;
        }

        int test_constructor() {
            if(test_constructor_templated<float>() != 0) {
                return -1;
            }

            if(test_constructor_templated<double>() != 0) {
                return -1;
            }

            return 0;
        }
    } 
}

