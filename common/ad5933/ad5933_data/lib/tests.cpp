#include "ad5933/data/data.hpp"

#include "ad5933/data/tests.hpp"

namespace AD5933 {
    namespace Data_Tests {
        constexpr int16_t real = 15'000;
        constexpr int16_t imag = -9'000;

        constexpr uint8_t real_HB = static_cast<uint8_t>((real >> 8) & 0x00FF);
        constexpr uint8_t real_LB = static_cast<uint8_t>((real >> 0) & 0x00FF);
        constexpr uint8_t imag_HB = static_cast<uint8_t>((imag >> 8) & 0x00FF);
        constexpr uint8_t imag_LB = static_cast<uint8_t>((imag >> 0) & 0x00FF);

        constexpr AD5933::Data test_data { real, imag };
        constexpr std::array<uint8_t, 4> test_array_for_constructor {
            real_HB, real_LB,
            imag_HB, imag_LB,
        };

        int test_array_constructor() {
            constexpr AD5933::Data data_to_check_against {
                test_array_for_constructor
            };

            if constexpr(test_data.get_real_data() != data_to_check_against.get_real_data()) {
                return -1;
            }

            if constexpr(test_data.get_imag_data() != data_to_check_against.get_imag_data()) {
                return -1;
            }

            if constexpr(test_data.get_raw_magnitude<float>() != data_to_check_against.get_raw_magnitude<float>()) {
                return -1;
            }

            if constexpr(test_data.get_raw_phase<float>() != data_to_check_against.get_raw_phase<float>()) {
                return -1;  
            }

            if constexpr(test_data.get_raw_magnitude<double>() != data_to_check_against.get_raw_magnitude<double>()) {
                return -1;
            }

            if constexpr(test_data.get_raw_phase<double>() != data_to_check_against.get_raw_phase<double>()) {
                return -1;  
            }

            return 0;
        }

        template<typename T_Floating>
        int test_raw_magnitude_templated() {
            constexpr int64_t real_product = static_cast<int64_t>(real) * static_cast<int64_t>(real);
            constexpr int64_t imag_product = static_cast<int64_t>(imag) * static_cast<int64_t>(imag);
            constexpr int64_t sum = real_product + imag_product;
            constexpr T_Floating raw_magnitude_to_check_against = std::sqrt(static_cast<T_Floating>(sum));
            if constexpr(raw_magnitude_to_check_against != test_data.get_raw_magnitude<T_Floating>()) {
                return -1;
            }
            return 0;
        }

        int test_raw_magnitude() {
            if(test_raw_magnitude_templated<float>() != 0) {
                return -1;
            }
            
            if(test_raw_magnitude_templated<double>() != 0) {
                return -1;
            }

            return 0;
        }

        template<typename T_Floating>
        int test_raw_phase_templated() {
            constexpr T_Floating raw_phase_to_check_against = std::atan2(static_cast<T_Floating>(imag), static_cast<T_Floating>(real));
            if constexpr(raw_phase_to_check_against != test_data.get_raw_phase<T_Floating>()) {
                return -1;
            }

            return 0;
        }

        int test_raw_phase() {
            if(test_raw_phase_templated<float>() != 0) {
                return -1;
            }
            
            if(test_raw_phase_templated<double>() != 0) {
                return -1;
            }

            return 0;
        }
    }
}

