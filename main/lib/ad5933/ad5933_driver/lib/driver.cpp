#include <cstdint>
#include <algorithm>

#include "i2c/device.hpp"
#include "ad5933/reg_addrs/reg_addrs.hpp"
#include "ad5933/reg_addrs/maps.hpp"

#include "ad5933/driver/driver.hpp"

namespace AD5933 {
    const char* namespace_name = "AD5933::";
    Driver::Driver(const i2c_master_dev_handle_t device_handle) :
        I2C::Device<RegAddrs::RW, RegAddrs::RW_RO, REG_COUNT>(SLAVE_ADDRESS, device_handle)
    {}

    bool Driver::write_to_register_address_pointer(const RegAddrs::RW_RO register_address) const {
        uint8_t write_buffer[2] { static_cast<uint8_t>(CommandCodes::AddressPointer), static_cast<uint8_t>(register_address) };
        const int xfer_timeout_ms = 100;

        if(i2c_master_transmit(
            device_handle,
            write_buffer,
            sizeof(write_buffer),
            xfer_timeout_ms
        ) == ESP_OK) {
            return true;
        } else {
            return false;
        }
    }

    bool Driver::program_all_registers(const std::array<uint8_t, RW_REG_COUNT> &message) const {
        return block_write_to_register<RW_REG_COUNT, RegAddrs::RW::ControlHB>(message);
    }

    void Driver::print_all_registers() const {
        std::printf("%s%sprint_all_registers: \n", namespace_name, class_name);
        const auto all_registers = dump_all_registers();
        if(all_registers.has_value()) {
            std::for_each(all_registers.value().begin(), all_registers.value().end(), [index = 0](const uint8_t &e) mutable {
                std::printf("\t%-11s (0x%02X): 0x%02X\n", RegAddrs::reg_map[index].second, static_cast<uint8_t>(RegAddrs::reg_map[index].first), e);
                index++;
            });
        }
    }
}