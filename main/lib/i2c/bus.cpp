#include <iostream>

#include "esp_log.h"

#include "i2c/i2c.hpp"

#include "i2c/bus.hpp"

namespace I2C {
	const char* namespace_name = "I2C::";

    Bus::Bus(const gpio_num_t sda_io_num, const gpio_num_t scl_io_num) {
        bus_config.sda_io_num = sda_io_num;
        bus_config.scl_io_num = scl_io_num;
        ESP_ERROR_CHECK(
            i2c_new_master_bus(&bus_config, &bus_handle)
        );
    }

    bool Bus::scan_for_addr(const uint8_t addr_7_bit) {
        if(i2c_master_probe(bus_handle, addr_7_bit, -1) == ESP_OK) {
            std::printf("%s%sscan_for_addr: Found device at address: 0x%02X\n", namespace_name, class_name, addr_7_bit);
            return true;
        }
        return false;
    }

    void Bus::scan() {
        /* https://github.com/espressif/esp-idf/issues/12159#issuecomment-1706433041 */
        const uint8_t addr_starter_7_bit = 0x01;
        const uint8_t addr_stopper_7_bit = 0x7E;

        for(uint8_t i = addr_starter_7_bit; i < addr_stopper_7_bit; i++) {
            if(scan_for_addr(i) == true) {
                std::printf("%s%sscan Found device at address: 0x%02X\n", namespace_name, class_name, i);
                device_addrs.push_back(i);
            }
        }
    }

    void Bus::config_print() const {
        std::printf("\n\n\n");
        std::printf("%s%sconfig_print:\n", namespace_name, class_name);
        std::printf("\ti2c_port: %d\n", bus_config.i2c_port);
        std::printf("\tsda_io_num: %d\n", static_cast<int>(bus_config.sda_io_num));
        std::printf("\tscl_io_num: %d\n", static_cast<int>(bus_config.scl_io_num));
        std::printf("\tclk_source: %d\n", bus_config.clk_source);
        std::printf("\tglitch_ignore_cnt: %d\n", bus_config.glitch_ignore_cnt);
        std::printf("\tintr_priority: %d\n", bus_config.intr_priority);
        std::printf("\ttrans_queue_depth: %d\n", bus_config.trans_queue_depth);
        std::printf("\tenable_internal_pullup: %d\n", bus_config.flags.enable_internal_pullup);
        std::printf("\n\n\n");
    }

    void Bus::devices_print_addrs() const {
        uint32_t i = 0;
        for(const auto addr: device_addrs) {
            std::printf("%s%sdevices_print_addrs: device_addrs[%lu]: 0x%02X\n", namespace_name, class_name, i, addr);
            i++;
        }
    }

    bool Bus::device_add_wo_scan(const uint16_t device_address) {
        i2c_master_dev_handle_t device_handle;
		const i2c_device_config_t device_config = {
			.dev_addr_length = I2C_ADDR_BIT_LEN_7,
			.device_address = device_address,
			.scl_speed_hz = 400'000u
		};

        ESP_ERROR_CHECK(
            i2c_master_bus_add_device(bus_handle, 
                &device_config,
                &device_handle
            )
        );
        active_device_handles[device_address] = device_handle;
        std::printf("%s%sdevice_add_wo_scan: Added device at address: 0x%02X to I2C Bus port: %u\n", namespace_name, class_name, device_address, bus_config.i2c_port);
        return true;
    }

    bool Bus::device_add(const uint16_t device_address) {
        if(scan_for_addr(device_address) == true) {
            return device_add_wo_scan(device_address);
        }
        return false;
    }

	void Bus::device_remove(const uint16_t device_address) {
        std::printf("%s%sdevice_remove: Removing device at address: 0x%02X to I2C Bus port: %u\n", namespace_name, class_name, device_address, bus_config.i2c_port);
        ESP_ERROR_CHECK(i2c_master_bus_rm_device(active_device_handles.at(device_address)));
        active_device_handles.erase(device_address);
    }

    Bus::~Bus() {
        for(const auto &device_handle: active_device_handles) {
            ESP_ERROR_CHECK(i2c_master_bus_rm_device(device_handle.second));
            std::printf("%s%s~Bus: Removed device at address: 0x%02X from I2CBus port: %u\n", namespace_name, class_name, device_handle.first, bus_config.i2c_port);
        }
        ESP_ERROR_CHECK(i2c_del_master_bus(bus_handle));
        std::printf("%s%s~Bus: Deleted master bus from I2CBus port: %u\n", namespace_name, class_name, bus_config.i2c_port);
    }
}
