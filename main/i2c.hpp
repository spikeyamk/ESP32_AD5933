#pragma once

#include <iostream>
#include <map>
#include <vector>
#include <optional>
#include <algorithm>
#include <thread>
#include <chrono>

#include "esp_log.h"
#include "driver/i2c_master.h"

#include "trielo/trielo.hpp"
#include "util.hpp"

class I2CBus {
public:
private:
	i2c_master_bus_config_t bus_config {
		.i2c_port = I2C_NUM_0,
		.sda_io_num = static_cast<gpio_num_t>(6),
		.scl_io_num = static_cast<gpio_num_t>(7),
		.clk_source = I2C_CLK_SRC_DEFAULT,
		.glitch_ignore_cnt = 7,
		.intr_priority = 0,
		.trans_queue_depth = 0,
		.flags {
			.enable_internal_pullup = 0
		}
	};

    i2c_master_bus_handle_t bus_handle;
public:
	std::vector<uint16_t> device_addrs;
	std::map<uint16_t, i2c_master_dev_handle_t> active_device_handles;

	I2CBus() {
		ESP_ERROR_CHECK(
			i2c_new_master_bus(&bus_config, &bus_handle)
		);
	}

    I2CBus(const gpio_num_t sda_io_num, const gpio_num_t scl_io_num) {
        bus_config.sda_io_num = sda_io_num;
        bus_config.scl_io_num = scl_io_num;
		ESP_ERROR_CHECK(
			i2c_new_master_bus(&bus_config, &bus_handle)
		);
    }

	bool scan_for_addr(const uint8_t addr_7_bit) {
		if(i2c_master_probe(bus_handle, addr_7_bit, -1) == ESP_OK) {
			std::printf("I2CBus: Found device at address: 0x%02X\n", addr_7_bit);
			return true;
		}
		return false;
	}

	void scan() {
		/* https://github.com/espressif/esp-idf/issues/12159#issuecomment-1706433041 */
        const uint8_t addr_starter_7_bit = 0x01;
		const uint8_t addr_stopper_7_bit = 0x7E;

    	for(uint8_t i = addr_starter_7_bit; i < addr_stopper_7_bit; i++) {
			if(i2c_master_probe(bus_handle, i, -1) == ESP_OK) {
				std::printf("I2CBus: Found device at address: 0x%02X\n", i);
				device_addrs.push_back(i);
			}
		}
	}

    void config_print() const {
		std::printf("\n\n\n");
		std::printf("I2CBus: config:\n");
        std::printf("i2c_port: %d\n", bus_config.i2c_port);
        std::printf("sda_io_num: %d\n", static_cast<int>(bus_config.sda_io_num));
        std::printf("scl_io_num: %d\n", static_cast<int>(bus_config.scl_io_num));
        std::printf("clk_source: %d\n", bus_config.clk_source);
        std::printf("glitch_ignore_cnt: %d\n", bus_config.glitch_ignore_cnt);
        std::printf("intr_priority: %d\n", bus_config.intr_priority);
        std::printf("trans_queue_depth: %d\n", bus_config.trans_queue_depth);
        std::printf("enable_internal_pullup: %d\n", bus_config.flags.enable_internal_pullup);
		std::printf("\n\n\n");
    }

	void devices_print_addrs() const {
		uint32_t i = 0;
		for(const auto addr: device_addrs) {
			std::printf("I2CBus: device_addrs[%lu]: 0x%02X\n", i, addr);
			i++;
		}
	}

	bool device_add_wo_scan(const uint16_t device_address) {
		i2c_device_config_t device_config = {
			.dev_addr_length = I2C_ADDR_BIT_LEN_7,
			.device_address = device_address,
			.scl_speed_hz = 100'000u
		};
		i2c_master_dev_handle_t device_handle;
		ESP_ERROR_CHECK(
			i2c_master_bus_add_device(bus_handle, 
				&device_config,
				&device_handle
			)
		);
		active_device_handles[device_address] = device_handle;
		std::printf("I2CBus: Added device at address: 0x%02X to I2CBus port: %u\n", device_address, bus_config.i2c_port);
		return true;
	}

	bool device_add(const uint16_t device_address) {
		if(scan_for_addr(device_address) == true) {
			i2c_device_config_t device_config = {
				.dev_addr_length = I2C_ADDR_BIT_LEN_7,
				.device_address = device_address,
				.scl_speed_hz = 100'000u
			};
			i2c_master_dev_handle_t device_handle;
			ESP_ERROR_CHECK(
				i2c_master_bus_add_device(bus_handle, 
					&device_config,
					&device_handle
				)
			);
			active_device_handles[device_address] = device_handle;
			std::printf("I2CBus: Added device at address: 0x%02X to I2CBus port: %u\n", device_address, bus_config.i2c_port);
			return true;
		}
		return false;
	}

    ~I2CBus() {
		for(const auto &device_handle: active_device_handles) {
			ESP_ERROR_CHECK(i2c_master_bus_rm_device(device_handle.second));
			std::printf("I2CBus: Removed device at address: 0x%02X from I2CBus port: %u\n", device_handle.first, bus_config.i2c_port);
		}
		ESP_ERROR_CHECK(i2c_del_master_bus(bus_handle));
		std::printf("I2CBus: Deleted master bus from I2CBus port: %u\n", bus_config.i2c_port);
    }
};

class RegisterAddr {
public:
	const uint8_t value;
	const std::optional<int> haha = std::nullopt;
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

class I2CDevice {
public:
	const i2c_master_dev_handle_t device_handle;
	const RegisterAddr lowest_register_address;
	const RegisterAddr highest_register_address;

	I2CDevice(
		const i2c_master_dev_handle_t &device_handle,
		const uint8_t lowest_register_address,
		const uint8_t highest_register_address
	) :
		device_handle{device_handle},
		lowest_register_address{lowest_register_address},
		highest_register_address{highest_register_address}
	{}

	bool register_address_exists(const RegisterAddr register_address) const {
		if(register_address >= lowest_register_address
		&& register_address <= highest_register_address) {
			return true;
		}
		return false;
	}

	virtual std::optional<uint8_t> read_register(const RegisterAddr register_address) const {
		uint8_t read_buffer;
		uint8_t write_buffer = register_address.unwrap();
		const int xfer_timeout_ms = -1;
		ESP_ERROR_CHECK(
			i2c_master_transmit_receive(
				device_handle,
				&write_buffer,
				sizeof(write_buffer),
				&read_buffer,
				sizeof(read_buffer),
				xfer_timeout_ms
			)
		);

		return std::optional {read_buffer};
	}

	bool register_address_range_exists(
		const RegisterAddr register_starting_address,
		const size_t n_bytes
	) const {
		if(register_starting_address.unwrap() + n_bytes <= highest_register_address.unwrap() + 1) {
			return true;
		}
		return false;
	}

	virtual std::optional<std::vector<uint8_t>> block_read_register(
		const RegisterAddr register_starting_address,
		const uint8_t n_bytes
	) const {
		if(register_address_range_exists(register_starting_address, n_bytes) == false) {
			return std::nullopt;
		}

		const int xfer_timeout_ms = -1;
		std::vector<uint8_t> tmp_vector;

		const uint8_t block_size = 10;
		for(uint8_t i = 0; i < n_bytes; i += block_size) {
			uint8_t write_buffer = register_starting_address.unwrap();
			uint8_t read_buffer[block_size] = {0x00, 0x00, 0x00, 0x00, 0x00};
			uint8_t remaining = n_bytes - i;

			ESP_ERROR_CHECK(
				i2c_master_transmit_receive(
					device_handle,
					&write_buffer,
					sizeof(write_buffer),
					read_buffer,
					std::min(block_size, remaining), // Ensure not to read more than remaining data.
					xfer_timeout_ms
				)
			);

			for(uint8_t j = 0; j < std::min(block_size, remaining); ++j) {
				tmp_vector.push_back(read_buffer[j]);
			}
		}

		return std::optional<std::vector<uint8_t>>(tmp_vector);
	}

	virtual void dump_all_registers() const {
		const auto ret = block_read_register(lowest_register_address, highest_register_address.unwrap() - lowest_register_address.unwrap() + 1);
		if(ret.has_value()) {
			uint32_t i = 0;
			for(const auto register_value: ret.value()) {
				std::printf("Register[0x%02lX]: 0x%02X\n", i + lowest_register_address.unwrap(), register_value);
				i++;
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
			}
		}
	}

	bool write_to_register_wo_check(const RegisterAddr_RW address, const uint8_t value) const {
		uint8_t write_buffer[2] { address.unwrap(), value};
		const int xfer_timeout_ms = -1;
		ESP_ERROR_CHECK(
			i2c_master_transmit(
				device_handle,
				write_buffer,
				sizeof(write_buffer),
				xfer_timeout_ms
			)
		);
		return true;
	}

	bool write_to_register(const RegisterAddr_RW address, const uint8_t value) const {
		write_to_register_wo_check(address, value);
		const std::optional<uint8_t> tmp = read_register(address);
		if(tmp.has_value() && tmp.value() == value) {
			return true;
		} else {
			return false;
		}
	}
};
