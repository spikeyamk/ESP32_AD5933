#include <cstdint>
#include <algorithm>

#include "i2c/device.hpp"
#include "ad5933/register_addrs.hpp"

#include "ad5933/driver/driver.hpp"

namespace AD5933 {
    const char* namespace_name = "AD5933::";
    Driver::Driver(I2C::Bus &bus, const i2c_master_dev_handle_t device_handle) :
        I2C::Device<RegsRW, RegsRW_RO, REG_COUNT>(SLAVE_ADDRESS, bus, device_handle)
    {}

    bool Driver::write_to_register_address_pointer(const RegsRW_RO register_address) const {
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
        return block_write_to_register<RW_REG_COUNT>(RegsRW::ControlHB, message);
    }

    void Driver::print_all_registers() const {
        std::printf("%s%sprint_all_registers: \n", namespace_name, class_name);
        const auto all_registers = dump_all_registers();
        if(all_registers.has_value()) {
            std::for_each(all_registers.value().begin(), all_registers.value().end(),
            [index = static_cast<int>(AD5933::RegsRW_RO::ControlHB)](const uint8_t &e) mutable {
                std::printf("\tRegister[0x%02X]: 0x%02X\n", index, e);
                if(index == static_cast<uint8_t>(AD5933::RegsRW_RO::Status)) {
                    index = static_cast<int>(AD5933::RegsRW_RO::TempDataHB);
                } else if((index + 1) > static_cast<int>(AD5933::RegsRW_RO::SetCyclesLB)) {
                    index = static_cast<int>(AD5933::RegsRW_RO::Status);
                } else {
                    index++;
                }
            });
        }
    }
}