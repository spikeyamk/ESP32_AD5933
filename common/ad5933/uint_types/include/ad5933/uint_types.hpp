#pragma once

#include <iostream>
#include <bitset>
#include <cstdint>
#include <cassert>

namespace AD5933 {
    template<uint32_t min> 
    class uint_freq_t {
    protected:
        uint32_t value { min };
    public:
        static constexpr uint32_t max { 100'000 };
        uint_freq_t() = delete;

        explicit constexpr uint_freq_t(const uint32_t in_value) :
            value { 
                [](const uint32_t in_value) -> uint32_t {
                    assert((in_value >= min) && (in_value <= max));
                    return in_value;
                }(in_value)
            }
        {}

        constexpr float to_float() const {
            return static_cast<float>(value); // Convert uint32_t to float
        }

        constexpr uint32_t unwrap() const {
            return value;
        }

        bool operator!=(const uint_freq_t<min> &other) const {
            return other.value != value;
        }
    };

    class uint_startfreq_t : public uint_freq_t<1'000> {
    public:
        explicit constexpr uint_startfreq_t(const uint32_t value) :
            uint_freq_t<1'000> { value }
        {}

        friend std::ostream& operator<<(std::ostream& os, const uint_startfreq_t& num) {
            os << num.value;
            return os;
        }
    };

    class uint_incfreq_t : public uint_freq_t<1> {
    public:
        explicit constexpr uint_incfreq_t(const uint32_t value) :
            uint_freq_t<1> { value }
        {}

        friend std::ostream& operator<<(std::ostream& os, const uint_incfreq_t& num) {
            os << num.value;
            return os;
        }
    };

    class uint9_t {
    private:
        uint16_t value : 9;
    public:
        // Overload the << operator to print a uint8_t
        friend std::ostream& operator<<(std::ostream& os, const uint9_t& num) {
            os << num.value;
            return os;
        }

        // Default constructor
        constexpr uint9_t() : value {} {}

        constexpr uint9_t(const uint16_t in_value) :
            value { 
                [](const uint16_t in_value) -> uint16_t {
                    assert(in_value < 512);
                    return in_value;
                }(in_value) 
            }
        {}

        // Copy constructor
        constexpr uint9_t(const uint9_t& other) : value(other.value) {}

        explicit operator uint8_t() const {
            return static_cast<uint8_t>(value & 0xFF); // Extract the least significant 8 bits
        }

        // Increment operator (postfix ++)
        constexpr uint9_t operator++(int) {
            uint9_t temp = *this;
            value = (value + 1) & 0x1FF; // Increment and ensure the value is within 9 bits
            return temp;
        }

        // Decrement operator (postfix --)
        constexpr uint9_t operator--(int) {
            uint9_t temp = *this;
            value = (value - 1) & 0x1FFu; // Decrement and ensure the value is within 9 bits
            return temp;
        }

        // Assignment operator
        constexpr uint9_t& operator=(const uint16_t& newValue) {
            value = newValue & 0x1FF; // Ensure the value is within 9 bits
            return *this;
        }

        // Increment operator (prefix ++)
        constexpr uint9_t& operator++() {
            value = (value + 1) & 0x1FFu; // Increment and ensure the value is within 9 bits
            return *this;
        }

        // Decrement operator (prefix --)
        constexpr uint9_t& operator--() {
            value = (value - 1) & 0x1FFu; // Decrement and ensure the value is within 9 bits
            return *this;
        }

        // Equal to operator (==)
        constexpr bool operator==(const uint9_t& other) const {
            return value == other.value;
        }

        // Not equal to operator (!=)
        constexpr bool operator!=(const uint9_t& other) const {
            return value != other.value;
        }

        // Less than operator (<)
        constexpr bool operator<(const uint9_t& other) const {
            return value < other.value;
        }

        // Less than or equal to operator (<=)
        constexpr bool operator<=(const uint9_t& other) const {
            return value <= other.value;
        }

        // Greater than operator (>)
        constexpr bool operator>(const uint9_t& other) const {
            return value > other.value;
        }

        // Greater than or equal to operator (>=)
        constexpr bool operator>=(const uint9_t& other) const {
            return value >= other.value;
        }

        // Right-shift operator (>>)
        constexpr uint9_t operator>>(unsigned int shift) const {
            uint9_t result;
            result.value = value >> shift;
            return result;
        }

        // Left-shift operator (<<)
        constexpr uint9_t operator<<(unsigned int shift) const {
            uint9_t result;
            result.value = (value << shift) & 0x1FFu; // Ensure the value is within 9 bits
            return result;
        }

        // Compound assignment operators
        // Addition assignment operator (+=)
        constexpr uint9_t& operator+=(const uint9_t& other) {
            value = (value + other.value) & 0x1FFu; // Add and ensure the value is within 9 bits
            return *this;
        }

        // Subtraction assignment operator (-=)
        constexpr uint9_t& operator-=(const uint9_t& other) {
            value = (value - other.value) & 0x1FFu; // Subtract and ensure the value is within 9 bits
            return *this;
        }

        // Bitwise AND assignment operator (&=)
        constexpr uint9_t& operator&=(const uint9_t& other) {
            value = value & other.value;
            return *this;
        }
        
        constexpr uint9_t& operator&(uint9_t& other) const {
            other &= value;
            return other;
        }

        // Bitwise OR assignment operator (|=)
        constexpr uint9_t& operator|=(const uint9_t& other) {
            value = value | other.value;
            return *this;
        }

        // Bitwise XOR assignment operator (^=)
        constexpr uint9_t& operator^=(const uint9_t& other) {
            value = value ^ other.value;
            return *this;
        }

        // Right-shift assignment operator (>>=)
        constexpr uint9_t& operator>>=(unsigned int shift) {
            value = value >> shift;
            return *this;
        }

        // Left-shift assignment operator (<<=)
        constexpr uint9_t& operator<<=(unsigned int shift) {
            value = (value << shift) & 0x1FFu; // Ensure the value is within 9 bits
            return *this;
        }

        // Lower 8 bits extraction function
        constexpr std::bitset<8> get_bit_7_to_0_LB() const {
            return std::bitset<8>(static_cast<uint8_t>((value & 0x00'FFu)));
        }

        // Bit 9 value retrieval function
        constexpr std::bitset<8> get_bit_8_MSB() const {
            return std::bitset<8>( (value & 0x01'00u) >> 8 );
        }

        constexpr uint16_t unwrap() const {
            return value;
        }
    };
}

std::ostream& operator<<(std::ostream& os, AD5933::uint_startfreq_t &obj);
std::ostream& operator<<(std::ostream& os, AD5933::uint_incfreq_t &obj);
