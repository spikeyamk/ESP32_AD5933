#include "ad5933/config/maps.hpp"

namespace AD5933 {
    namespace Masks {
        template<typename T_Enum, typename T_Map>
        const char* get_map_str(const T_Enum e, const T_Map &map) {
            for(const auto &i: map) {
                if(i.first == e) {
                    return i.second;
                }
            }
            return nullptr;
        }
    }
}

std::ostream& operator<<(std::ostream& os, const AD5933::Masks::Or::Ctrl::HB::Command obj) {
    os << AD5933::Masks::get_map_str(obj, AD5933::Masks::Or::Ctrl::HB::command_map);
    return os;
}

std::ostream& operator<<(std::ostream& os, const AD5933::Masks::Or::Ctrl::HB::VoltageRange obj) {
    os << AD5933::Masks::get_map_str(obj, AD5933::Masks::Or::Ctrl::HB::voltage_map);
    return os;
}

std::ostream& operator<<(std::ostream& os, const AD5933::Masks::Or::Ctrl::HB::PGA_Gain obj) {
    os << AD5933::Masks::get_map_str(obj, AD5933::Masks::Or::Ctrl::HB::pga_gain_map);
    return os;
}

std::ostream& operator<<(std::ostream& os, const AD5933::Masks::Or::Ctrl::LB::SysClkSrc obj) {
    os << AD5933::Masks::get_map_str(obj, AD5933::Masks::Or::Ctrl::LB::sysclk_src_map);
    return os;
}

std::ostream& operator<<(std::ostream& os, const AD5933::Masks::Or::SettlingTimeCyclesHB::Multiplier obj) {
    os << AD5933::Masks::get_map_str(obj, AD5933::Masks::Or::SettlingTimeCyclesHB::multiplier_map);
    return os;
}

