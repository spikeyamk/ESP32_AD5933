#pragma once

#include <cstdint>
#include <array>
#include <optional>

#include "i2c/device.hpp"
#include "ad5933/register_addrs.hpp"

namespace AD5933 {
    extern const char* namespace_name;
    constexpr size_t REG_COUNT = 19;
    constexpr size_t RW_REG_COUNT = static_cast<size_t>(static_cast<uint8_t>(RegsRW_RO::SetCyclesLB) - static_cast<uint8_t>(RegsRW_RO::ControlHB) + 1);
    constexpr uint8_t SLAVE_ADDRESS = 0x0D; 
	constexpr uint32_t POW_2_27 = 134217728ul;
    enum class CommandCodes {
		BlockWrite     = 0b1010'0000,
		BlockRead      = 0b1010'0001,
		AddressPointer = 0b1011'0000,
    };

    class Driver : public I2C::Device<RegsRW, RegsRW_RO, REG_COUNT> {
    private:
        static constexpr char class_name[] = "Driver::";
    public:
        Driver(I2C::Bus &bus, const i2c_master_dev_handle_t device_handle);
        bool write_to_register_address_pointer(const RegsRW_RO register_address) const;

        virtual std::optional<uint8_t> read_register(const RegsRW_RO reg) const override {
            if(write_to_register_address_pointer(reg) == false) {
                return std::nullopt;
            }

            uint8_t read_buffer;	
            const int xfer_timeout_ms = 100;
            if(i2c_master_receive(
                device_handle,
                &read_buffer,
                sizeof(read_buffer),
                xfer_timeout_ms
            ) != ESP_OK) {
                return std::nullopt;
            }

            return read_buffer;
        }

        template<size_t n_bytes>
        std::optional<std::array<uint8_t, n_bytes>> block_read_register(const RegsRW_RO reg) const {
            if(write_to_register_address_pointer(reg) == false) {
                return std::nullopt;
            }

            const uint8_t write_buffer[2] = { static_cast<uint8_t>(CommandCodes::BlockRead), n_bytes };
            std::array<uint8_t, n_bytes> read_buffer;
            const int xfer_timeout_ms = 100;
            if(i2c_master_transmit_receive(
                device_handle,
                write_buffer,
                sizeof(write_buffer),
                read_buffer.data(),
                read_buffer.size(),
                xfer_timeout_ms
            ) != ESP_OK) {
                return std::nullopt;
            }

		    return read_buffer;
        }

        virtual std::optional<std::array<uint8_t, REG_COUNT>> dump_all_registers() const override {
            const auto rw_ret = block_read_register<RW_REG_COUNT>(RegsRW_RO::ControlHB);
            if(rw_ret.has_value() == false) {
                return std::nullopt;
            }

            const auto status_ret = read_register(RegsRW_RO::Status);
            if(status_ret.has_value() == false) {
                return std::nullopt;
            }

            constexpr auto ro_n_bytes = static_cast<size_t>(static_cast<uint8_t>(RegsRW_RO::ImagDataLB) - static_cast<uint8_t>(RegsRW_RO::TempDataHB) + 1);
            const auto ro_ret = block_read_register<ro_n_bytes>(RegsRW_RO::TempDataHB);
            if(ro_ret.has_value() == false) {
                return std::nullopt;
            }

            std::array<uint8_t, 19> ret_array;
            std::copy(rw_ret.value().begin(), rw_ret.value().end(), ret_array.begin());
            ret_array[12] = status_ret.value();
            std::copy(ro_ret.value().begin(), ro_ret.value().end(), ret_array.begin() + 13);
            return ret_array;
        }

        template<size_t n_bytes>
        bool block_write_to_register(const RegsRW reg, const std::array<uint8_t, n_bytes> &message) const {
            static_assert(n_bytes <= 12, "n_bytes must be less than or equal to 12");
            if(write_to_register_address_pointer(RegsRW_RO(static_cast<uint8_t>(reg))) == false) {
                return false;
            }

            std::array<uint8_t, n_bytes + 2> write_buffer { 
                static_cast<uint8_t>(CommandCodes::BlockWrite),
                static_cast<uint8_t>(message.size())
            };
            std::copy(message.begin(), message.end(), write_buffer.begin() + 2);
            const int xfer_timeout_ms = 100;

            if(i2c_master_transmit(
                device_handle,
                write_buffer.data(),
                write_buffer.size(),
                xfer_timeout_ms
            ) == ESP_OK) {
                return true;
            } else {
                return false;
            }
        }

        bool program_all_registers(const std::array<uint8_t, RW_REG_COUNT> &message) const;
        void print_all_registers() const;
    };
}