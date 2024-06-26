#include "ad5933/masks/maps.hpp"

namespace AD5933 {
    namespace Masks {

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

