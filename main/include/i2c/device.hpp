#pragma once

#include <iostream>
#include <optional>
#include <cstdint>
#include <array>

#include "esp_log.h"
#include "driver/i2c_master.h"

#include "i2c/bus.hpp"

#include "i2c/register_addr.hpp"

namespace I2C {
    template<typename RegRW_T, typename RegRW_RO_T, size_t reg_count>
    class Device {
    public:
        static constexpr char class_name[] = "Device::";
        const uint16_t slave_addr;
        Bus& bus;
        const i2c_master_dev_handle_t device_handle;
        Device(const uint16_t slave_addr, Bus &bus, const i2c_master_dev_handle_t device_handle) :
            slave_addr{slave_addr},
            bus{bus},
            device_handle{device_handle}
        {}

        ~Device() {
            std::printf("%s%s~Device: Removing slave device on addr: 0x%02X\n", namespace_name, class_name, slave_addr);
            bus.device_remove(slave_addr);
        }

        virtual std::optional<uint8_t> read_register(const RegRW_RO_T reg) const {
            uint8_t read_buffer;
            const uint8_t write_buffer = static_cast<uint8_t>(reg);
            const int xfer_timeout_ms = 100;
            const auto ret = i2c_master_transmit_receive(
                device_handle,
                &write_buffer,
                sizeof(write_buffer),
                &read_buffer,
                sizeof(read_buffer),
                xfer_timeout_ms
            );
            
            if(ret == ESP_OK) {
                return read_buffer;
            } else {
                ESP_ERROR_CHECK_WITHOUT_ABORT(ret);
                return std::nullopt;
            }
        }

        virtual bool write_to_register(const RegRW_T reg, const uint8_t value) const {
            const uint8_t write_buffer[2] { static_cast<uint8_t>(reg), value};
            const int xfer_timeout_ms = 100;
            esp_err_t ret = i2c_master_transmit(
                device_handle,
                write_buffer,
                sizeof(write_buffer),
                xfer_timeout_ms
            );
            if(ret == ESP_OK) {
                return true;
            } else {
                ESP_ERROR_CHECK_WITHOUT_ABORT(ret);
                return false;
            }
        }

        virtual bool write_to_register_with_check(const RegRW_T reg, const uint8_t value) const {
            if(write_to_register(reg, value) == false) {
                return false;
            }
            const auto read = read_register(RegRW_RO_T(static_cast<uint8_t>(reg)));
            if(read.has_value() || (read.value() != value)) {
                return false;
            }
            return true;
        }

        virtual std::optional<std::array<uint8_t, reg_count>> dump_all_registers() const = 0;
    };
}
