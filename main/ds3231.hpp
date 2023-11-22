#pragma once

#include <iostream>
#include <optional>
#include <thread>
#include <chrono>
#include <atomic>
#include <memory>

#include "util.hpp"
#include "trielo/trielo.hpp"
#include "i2c.hpp"

class DS3231 : public I2CDevice {
public:
	std::optional<uint8_t> secs = std::nullopt;
	std::optional<uint8_t> mins = std::nullopt;
	std::optional<TaskHandle_t> load_secs_n_mins_task_handle = std::nullopt;
	std::atomic<bool> load_task_enable = false;
	std::optional<std::thread> load_thread_handle = std::nullopt;

	static constexpr RegisterAddr SECS_REGISTER { 0x00 };
	static constexpr RegisterAddr MINS_REGISTER { 0x01 };
	static const uint8_t SLAVE_ADDRESS = 0x68;

	DS3231(const i2c_master_dev_handle_t &device_handle) :
		I2CDevice(device_handle, 0x00, 0x12)
	{}

	static void load_secs_n_mins_cb(DS3231* self) {
		while(self->load_task_enable == true) {
			self->secs = self->read_register(SECS_REGISTER);
			self->mins = self->read_register(MINS_REGISTER);
			self->print_time();
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}
	} friend void load_secs_n_mins_cb(void *arg);

	/* Function that creates a task. */
	bool create_load_secs_n_mins_task() {
		if(load_task_enable == true) {
			return false;
		}
		load_task_enable = true;
		std::thread tmp_thread(load_secs_n_mins_cb, this);
		load_thread_handle = std::move(tmp_thread);
		return true;
	}

	void print_time() const {
		std::printf("DS3231: time: %02X:%02X\n", mins.value_or(0xFF), secs.value_or(0xFF));
	}

	bool delete_load_secs_n_mins_task() {
		if(load_task_enable == false && load_thread_handle.has_value() == false) {
			return false;
		}

		load_task_enable = false;
		if(load_thread_handle.value().joinable() == false) {
			return false;
		}

		load_thread_handle.value().join();
		secs = std::nullopt;
		mins = std::nullopt;
		load_thread_handle = std::nullopt;
		return true;
	}

	~DS3231() {
		if(delete_load_secs_n_mins_task() == true) {
			std::printf("DS3231: Successfully deleted the load_secs_n_mins_task\n");
		} else {
			std::printf("DS3231: Failed to delete the load_secs_n_mins_task\n");
		}
	}
};

std::unique_ptr<DS3231> run_ds3231(I2CBus &i2c_bus) {
	i2c_bus.device_add(DS3231::SLAVE_ADDRESS);
    std::unique_ptr<DS3231> ds3231 = std::make_unique<DS3231>(i2c_bus.active_device_handles[DS3231::SLAVE_ADDRESS]);
	ds3231->dump_all_registers();
	ds3231->create_load_secs_n_mins_task();
	return ds3231;
}