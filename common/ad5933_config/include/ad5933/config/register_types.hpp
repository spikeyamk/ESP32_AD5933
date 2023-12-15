#pragma once

#include <iostream>
#include <bitset>

namespace AD5933 {
    class Register8_t {
    public:
        std::bitset<8> HB { 0 };

        constexpr Register8_t(const std::bitset<8> in_HB) :
            HB {in_HB}
        {}

        constexpr Register8_t() = default;
    };

    class Register16_t : public Register8_t {
    public:
        std::bitset<8> LB { 0 };

        constexpr Register16_t(const std::bitset<8> in_HB, const std::bitset<8> in_LB) :
            Register8_t {in_HB},
            LB {in_LB}
        {}

        constexpr Register16_t() = default;

        constexpr std::bitset<16> get_HB_LB_combined_bitset() const {
            return std::bitset<16> { (HB.to_ulong() << 8) | LB.to_ulong() };
        }
    };

    class Register24_t : public Register16_t {
    public:
        std::bitset<8> MB { 0 };

        constexpr Register24_t(const std::bitset<8> in_HB, const std::bitset<8> in_MB, const std::bitset<8> in_LB) :
            Register16_t {in_HB, in_LB},
            MB {in_MB}
        {}

        constexpr std::bitset<24> get_HB_MB_LB_combined_bitset() const {
            return std::bitset<24> { (HB.to_ulong() << 16 ) | (MB.to_ulong() << 8 ) | LB.to_ulong() };
        }

        constexpr Register24_t() = default;
    };
}