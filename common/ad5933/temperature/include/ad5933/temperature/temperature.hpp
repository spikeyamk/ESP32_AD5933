#pragma once

#include <cstdint>
#include <array>

namespace AD5933 {
    template<typename T_Floating>
    class Temperature {
    private:
        T_Floating value; 
    public:
        inline Temperature(const std::array<uint8_t, 2> &raw) :
            value { init_value(raw) }
        {}
    public:
        inline T_Floating get_value() const {
            return value;
        }
    private:
        inline T_Floating init_value(const std::array<uint8_t, 2> &raw) {
            const uint16_t temp_hb = static_cast<uint16_t>(raw[0]) << 8;
            const uint16_t temp_lb = static_cast<uint16_t>(raw[1]);
            const int16_t temperature = static_cast<int16_t>(temp_lb | temp_hb);
            if(temperature < 8192) {
                return static_cast<T_Floating>(temperature) / static_cast<T_Floating>(32.0);
            } else {
                return static_cast<T_Floating>(temperature - 16384) / static_cast<T_Floating>(32.0);
            }
        }
    };
}