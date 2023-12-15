#pragma once

#include <iostream>
#include <cstdint>
#include <bitset>

struct uint9_t {
    uint16_t value : 9;

    // Overload the << operator to print a uint8_t
    friend std::ostream& operator<<(std::ostream& os, const uint9_t& num) {
        os << num.value;
        return os;
    }

    // Default constructor
    constexpr uint9_t() : value {} {}

    constexpr uint9_t(const uint16_t in_value) :
		value { in_value }
	{}

    // Copy constructor
    constexpr uint9_t(const uint9_t& other) : value(other.value) {}

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
    constexpr std::bitset<1> get_bit_8_MSB() const {
        return std::bitset<1>( (value & 0x01'00u) >> 8 );
    }
};

struct uint24_t {
    uint32_t value : 24;

	uint24_t(const uint32_t in_value) :
		value { in_value }
	{}

    // Overload the << operator to print a uint24_t
    friend std::ostream& operator<<(std::ostream& os, const uint24_t& num) {
        os << num.value;
        return os;
    }

    // Default constructor
    constexpr uint24_t() : value {} {}

    // Copy constructor
    constexpr uint24_t(const uint24_t& other) : value(other.value) {}

    // Increment operator (postfix ++)
    constexpr uint24_t operator++(int) {
        uint24_t temp = *this;
        value = (value + 1) & 0xFFFFFFu; // Increment and ensure the value is within 24 bits
        return temp;
    }

    // Decrement operator (postfix --)
    constexpr uint24_t operator--(int) {
        uint24_t temp = *this;
        value = (value - 1) & 0xFFFFFFu; // Decrement and ensure the value is within 24 bits
        return temp;
    }

    // Assignment operator
    constexpr uint24_t& operator=(const uint32_t& newValue) {
        value = newValue & 0xFFFFFFu; // Ensure the value is within 24 bits
        return *this;
    }

    // Increment operator (prefix ++)
    constexpr uint24_t& operator++() {
        value = (value + 1) & 0xFFFFFFu; // Increment and ensure the value is within 24 bits
        return *this;
    }

    // Decrement operator (prefix --)
    constexpr uint24_t& operator--() {
        value = (value - 1) & 0xFFFFFFu; // Decrement and ensure the value is within 24 bits
        return *this;
    }

    // Equal to operator (==)
    constexpr bool operator==(const uint24_t& other) const {
        return value == other.value;
    }

    // Not equal to operator (!=)
    constexpr bool operator!=(const uint24_t& other) const {
        return value != other.value;
    }

    // Less than operator (<)
    constexpr bool operator<(const uint24_t& other) const {
        return value < other.value;
    }

    // Less than or equal to operator (<=)
    constexpr bool operator<=(const uint24_t& other) const {
        return value <= other.value;
    }

    // Greater than operator (>)
    constexpr bool operator>(const uint24_t& other) const {
        return value > other.value;
    }

    // Greater than or equal to operator (>=)
    constexpr bool operator>=(const uint24_t& other) const {
        return value >= other.value;
    }

    // Right-shift operator (>>)
    constexpr uint24_t operator>>(unsigned int shift) const {
        uint24_t result;
        result.value = value >> shift;
        return result;
    }

    // Left-shift operator (<<)
    constexpr uint24_t operator<<(unsigned int shift) const {
        uint24_t result;
        result.value = (value << shift) & 0xFFFFFFu; // Ensure the value is within 24 bits
        return result;
    }

    // Compound assignment operators

    // Addition assignment operator (+=)
    constexpr uint24_t& operator+=(const uint24_t& other) {
        value = (value + other.value) & 0xFFFFFFu; // Add and ensure the value is within 24 bits
        return *this;
    }

    // Subtraction assignment operator (-=)
    constexpr uint24_t& operator-=(const uint24_t& other) {
        value = (value - other.value) & 0xFFFFFFu; // Subtract and ensure the value is within 24 bits
        return *this;
    }

    // Bitwise AND assignment operator (&=)
    constexpr uint24_t& operator&=(const uint24_t& other) {
        value = value & other.value;
        return *this;
    }

    // Bitwise OR assignment operator (|=)
    constexpr uint24_t& operator|=(const uint24_t& other) {
        value = value | other.value;
        return *this;
    }

    // Bitwise XOR assignment operator (^=)
    constexpr uint24_t& operator^=(const uint24_t& other) {
        value = value ^ other.value;
        return *this;
    }

    // Right-shift assignment operator (>>=)
    constexpr uint24_t& operator>>=(unsigned int shift) {
        value = value >> shift;
        return *this;
    }

    // Left-shift assignment operator (<<=)
    constexpr uint24_t& operator<<=(unsigned int shift) {
        value = (value << shift) & 0xFFFFFFu; // Ensure the value is within 24 bits
        return *this;
    }

    // High Byte extraction function
    constexpr std::bitset<8> get_HB() const {
        return std::bitset<8>((value >> 16) & 0xFF);
    }

    // Middle Byte extraction function
    constexpr std::bitset<8> get_MB() const {
        return std::bitset<8>((value >> 8) & 0xFF);
    }

    // Low Byte extraction function
    constexpr std::bitset<8> get_LB() const {
        return std::bitset<8>(value & 0xFF);
    }
};

class Register8_t {
public:
	std::bitset<8> HB { 0 };

	constexpr Register8_t(const std::bitset<8> in_HB) :
		HB {in_HB}
	{}

	constexpr Register8_t() = default;

    friend std::ostream& operator<<(std::ostream& os, const Register8_t& reg) {
        os << "HB: " << reg.HB;
        return os;
    }
};

class Register16_t : public Register8_t {
public:
	std::bitset<8> LB { 0 };

	constexpr Register16_t(const std::bitset<8> in_HB, const std::bitset<8> in_LB) :
		Register8_t {in_HB},
		LB {in_LB}
	{}

	constexpr Register16_t() = default;

	std::bitset<16> get_HB_LB_combined_bitset() const {
		return std::bitset<16> { (HB.to_ulong() << 8) | LB.to_ulong() };
	}

    friend std::ostream& operator<<(std::ostream& os, const Register16_t& reg) {
        os << static_cast<const Register8_t&>(reg) << ", LB: " << reg.LB;
        return os;
    }
};

class Register24_t : public Register16_t {
public:
	std::bitset<8> MB { 0 };

	constexpr Register24_t(const std::bitset<8> in_HB, const std::bitset<8> in_MB, const std::bitset<8> in_LB) :
		Register16_t {in_HB, in_LB},
		MB {in_MB}
	{}

	std::bitset<24> get_HB_MB_LB_combined_bitset() const {
		return std::bitset<24> { (HB.to_ulong() << 16 ) | (MB.to_ulong() << 8 ) | LB.to_ulong() };
	}

	constexpr Register24_t() = default;

    friend std::ostream& operator<<(std::ostream& os, const Register24_t& reg) {
        os << static_cast<const Register16_t&>(reg) << ", MB: " << reg.MB;
        return os;
    }
};