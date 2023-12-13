#include "i2c/device.hpp"

namespace I2C {
    /*

    bool Device::register_address_exists(const RegisterAddr register_address) const {
        if(register_address >= lowest_register_address
        && register_address <= highest_register_address) {
            return true;
        }
        return false;
    }

    bool Device::register_address_range_exists(
        const RegisterAddr register_starting_address,
        const size_t n_bytes
    ) const {
        if(register_starting_address.unwrap() + n_bytes <= highest_register_address.unwrap() + 1) {
            return true;
        }
        return false;
    }

    bool Device::write_to_register_wo_check(const RegisterAddr_RW address, const uint8_t value) const {
        uint8_t write_buffer[2] { address.unwrap(), value};
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

    bool Device::write_to_register(const RegisterAddr_RW address, const uint8_t value) const {
        write_to_register_wo_check(address, value);
        const std::optional<uint8_t> tmp = read_register(address);
        if(tmp.has_value() && tmp.value() == value) {
            return true;
        } else {
            return false;
        }
    }
    */
}
