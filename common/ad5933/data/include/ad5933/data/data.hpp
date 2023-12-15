#pragma once

#include <array>
#include <cstdint>
#include <cmath>
#include <type_traits>

namespace AD5933 {
    class Data {
    private:
        int16_t real;
        int16_t imag;
    public:
        Data() = delete;

        constexpr inline Data(const int16_t real, const int16_t imag) :
            real{ real },
            imag{ imag }
        {}

        constexpr inline Data(const std::array<uint8_t, 4> &raw) :
            real { static_cast<int16_t>((static_cast<uint16_t>(raw[0]) << 8) | raw[1]) },
            imag { static_cast<int16_t>((static_cast<uint16_t>(raw[2]) << 8) | raw[3]) }
        {}
    public:
        constexpr inline int16_t get_real_data() const {
            return real;
        }

        constexpr inline int16_t get_imag_data() const {
            return imag;
        }

        template<typename T_Floating, typename = std::enable_if_t<std::is_same<T_Floating, float>::value || std::is_same<T_Floating, double>::value>>
        constexpr inline T_Floating get_raw_magnitude() const {
            static_assert(std::is_floating_point<T_Floating>::value, "T_Floating must be float or double");
            const int32_t real_product = (static_cast<int32_t>(real) * static_cast<int32_t>(real));
            const int32_t imag_product = (static_cast<int32_t>(imag) * static_cast<int32_t>(imag));
            const int32_t sum = real_product + imag_product;
            return std::sqrt(static_cast<T_Floating>(sum));
        }

        template<typename T_Floating, typename = std::enable_if_t<std::is_same<T_Floating, float>::value || std::is_same<T_Floating, double>::value>>
        constexpr inline T_Floating get_raw_phase() const {
            static_assert(std::is_floating_point<T_Floating>::value, "T_Floating must be float or double");
            return std::atan2(static_cast<T_Floating>(imag), static_cast<T_Floating>(real));
        }
    };
}