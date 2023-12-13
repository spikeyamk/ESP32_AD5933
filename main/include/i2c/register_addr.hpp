#pragma once

#include <cstdint>

namespace I2C {
    class RegisterAddr {
    private:
        const uint8_t value;
    public:
        inline uint8_t unwrap() const {
            return value;
        }

        constexpr explicit RegisterAddr(const uint8_t in_value) :
            value{ in_value }
        {}

        constexpr bool operator==(const RegisterAddr& rhs) const {
            if(value == rhs.value) {
                return true;
            } else {
                return false;
            }
        }

        constexpr bool operator>=(const RegisterAddr& rhs) const {
            if(value >= rhs.value) {
                return true;
            } else {
                return false;
            }
        }

        constexpr bool operator<=(const RegisterAddr& rhs) const {
            if(value <= rhs.value) {
                return true;
            } else {
                return false;
            }
        }
    };

    class RegisterAddr_RO : public RegisterAddr {
    public:
        constexpr explicit RegisterAddr_RO(const uint8_t in_value) :
            RegisterAddr{ in_value }
        {}
    };

    class RegisterAddr_RW : public RegisterAddr {
    public:
        constexpr explicit RegisterAddr_RW(const uint8_t in_value) :
            RegisterAddr{ in_value }
        {}
    };
}
