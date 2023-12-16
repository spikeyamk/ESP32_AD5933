#pragma once

#include <iostream>
#include <array>
#include <cstdint>
#include <optional>
#include <bitset>

#include "ad5933/driver/driver.hpp"

namespace AD5933 {
    class Extension {
    private:
    public:
        Driver& driver;

        Extension() = delete;
        Extension(Driver &driver);

        inline bool set_command(Masks::Or::Ctrl::HB::Command or_mask) const {
            return p_register_set_mask(RegAddrs::RW::ControlHB, static_cast<uint8_t>(or_mask), Masks::And::Ctrl::HB::Command);
        }

        inline bool has_status_condition(Masks::Or::Status or_mask) const {
            return p_register_has_mask(RegAddrs::RW_RO::Status, static_cast<uint8_t>(or_mask));
        }

        inline std::optional<std::array<uint8_t, 2>> read_temp_data() const {
            return driver.block_read_register<2, RegAddrs::RW_RO::TempDataHB>();
        }

        inline std::optional<std::array<uint8_t, 4>> read_impe_data() const {
            return driver.block_read_register<4, RegAddrs::RW_RO::RealDataHB>();
        }
        /*
        inline bool set_voltage_range(Masks::Or::Ctrl::HB::VoltageRange or_mask) const {
            return driver.register_set_mask(RegAddrs::RW::ControlHB, static_cast<uint8_t>(or_mask), Masks::And::Ctrl::HB::VoltageRange);
        }

        inline bool set_pga_gain(Masks::Or::Ctrl::HB::PGA_Gain or_mask) const {
            return driver.register_set_mask(RegAddrs::RW::ControlHB, static_cast<uint8_t>(or_mask), Masks::And::Ctrl::HB::PGA_Gain);
        }

        inline bool set_sysclk_src(Masks::Or::Ctrl::LB::SysClkSrc or_mask) const {
            return driver.register_set_mask(RegAddrs::RW::ControlLB, static_cast<uint8_t>(or_mask), Masks::And::Ctrl::LB::SysClkSrc);
        }

        inline bool set_settling_time_cycles_multiplier(Masks::Or::SettlingTimeCyclesHB::Multiplier or_mask) const {
            return driver.register_set_mask(RegAddrs::RW::SetCyclesHB, static_cast<uint8_t>(or_mask), Masks::And::SettlingTimeCyclesHB::Multiplier);
        }
        */
    private:
        inline bool p_register_has_mask(const RegAddrs::RW_RO reg, const uint8_t or_mask) const {
            const auto ret = driver.read_register(reg);
            if(ret.has_value() == false) {
                return false;
            }

            if((ret.value() & or_mask) == or_mask) {
                return true;
            } else {
                return false;
            }
        }

        inline bool p_register_set_mask(const RegAddrs::RW reg, const uint8_t set_mask, const std::bitset<8> clear_mask) const {
            const auto ret = driver.read_register(RegAddrs::RW_RO(static_cast<uint8_t>(reg)));
            if(ret.has_value() == false) {
                return false;
            }
            uint8_t message = ret.value();
            message &= static_cast<uint8_t>(~clear_mask.to_ulong());
            message |= set_mask;
            return driver.write_to_register(reg, message);
        }
    public:
        friend std::ostream& operator<<(std::ostream &os, const Extension &obj) {
            os << reinterpret_cast<const void*>(&obj);
            return os;
        }
    };
}