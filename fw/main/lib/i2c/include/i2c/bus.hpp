#pragma once

#include <vector>
#include <map>
#include <cstdint>

#include "driver/i2c_master.h"

namespace I2C {
	extern const char* namespace_name;

	class Bus {
	private:
		static constexpr char class_name[] { "Bus::" };
		i2c_master_bus_config_t bus_config {
			.i2c_port = I2C_NUM_0,
			.sda_io_num = gpio_num_t(6),
			.scl_io_num = gpio_num_t(7),
			.clk_source = I2C_CLK_SRC_DEFAULT,
			.glitch_ignore_cnt = 7,
			.intr_priority = 0,
			.trans_queue_depth = 0,
			.flags {
				.enable_internal_pullup = 0
			}
		};
		i2c_master_bus_handle_t bus_handle;
		Bus(const gpio_num_t sda_io_num = gpio_num_t(6), const gpio_num_t scl_io_num = gpio_num_t(7));
	public:
		std::vector<uint16_t> device_addrs;
		std::map<uint16_t, i2c_master_dev_handle_t> active_device_handles;
    public:
		static Bus& get_instance() {
        	static Bus instance {}; // Static instance created once
			return instance;
		}
		bool scan_for_addr(const uint8_t addr_7_bit);
		void scan();
		void config_print() const;
		void devices_print_addrs() const;
		bool device_add_wo_scan(const uint16_t device_address);
		bool device_add(const uint16_t device_address);
		void device_remove(const uint16_t device_handle);
		~Bus();
	};
}

