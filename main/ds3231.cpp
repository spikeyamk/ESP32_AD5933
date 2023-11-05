#include <iostream>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/i2c_master.h"

#include "util.hpp"
#include "ds3231.hpp"

namespace DS3231 {
	uint8_t secs = 0;
	uint8_t mins = 0;

	static void load_secs_n_mins(i2c_master_dev_handle_t device_handle) {
		const uint8_t zeroth_address = 0x00;
		const uint8_t write_buffer[1] = { zeroth_address };
		uint8_t read_buffer[2] = { 0x00, 0x00 };
		const int xfer_timeout_ms = -1;
		ESP_ERROR_CHECK(
			i2c_master_transmit_receive(
				device_handle,
				write_buffer,
				sizeof(write_buffer),
				read_buffer,
				sizeof(read_buffer),
				xfer_timeout_ms
			)
		);

		secs = read_buffer[0];
		mins = read_buffer[1];
	}

	static void run_cb(void *pvParameters) {
		i2c_master_bus_config_t i2c_bus_config = {
			.i2c_port = 0,
			.sda_io_num = static_cast<gpio_num_t>(6),
			.scl_io_num = static_cast<gpio_num_t>(7),
			.clk_source = I2C_CLK_SRC_DEFAULT,
			.glitch_ignore_cnt = 0,
			.intr_priority = 0,
			.trans_queue_depth = 0,
			.flags {
				.enable_internal_pullup = 0
			}
		};
		i2c_master_bus_handle_t bus_handle;
		ESP_ERROR_CHECK(
			i2c_new_master_bus(&i2c_bus_config, &bus_handle)
		);

		const uint8_t DS3231_addr = 0x68;
		i2c_device_config_t device_config = {
			.dev_addr_length = I2C_ADDR_BIT_LEN_7,
			.device_address = 0x00,
			.scl_speed_hz = I2C_CLK_SRC_DEFAULT
		};
		i2c_master_dev_handle_t device_handle;

		for(uint8_t i =0x01; i < 0x90; i++) {
			esp_err_t ret = i2c_master_probe(bus_handle, i, -1);
			if(ret == 0 && i == DS3231_addr) {
				printf("Found DS3231 on address: 0x%02x\n", i);
				device_config.device_address = DS3231_addr;
				ESP_ERROR_CHECK(
					i2c_master_bus_add_device(bus_handle, 
						&device_config,
						&device_handle
					)
				);
				const TickType_t xDelay = 100 / portTICK_PERIOD_MS;
				while(1) {
					vTaskDelay(xDelay);
					load_secs_n_mins(device_handle);
				}
			}
		}
		
		if(device_config.device_address == DS3231_addr) {
			printf("Removing DS3231 from master bus\n");
			ESP_ERROR_CHECK(i2c_master_bus_rm_device(device_handle));
		}

		printf("Deleting I2C master bus\n");
		ESP_ERROR_CHECK(i2c_del_master_bus(bus_handle));
	}

	/* Function that creates a task. */
	void run() {
		const uint32_t STACK_SIZE = 2*1024;
		const uint32_t priority = 1;

		/* Create the task, storing the handle. */
		xTaskCreate(
			run_cb, /* Function that implements the task. */
			"run_ds3231", /* Text name for the task. */
			STACK_SIZE,       /* Stack size in words, not bytes. */
			nullptr,        /* Parameter passed into the task. */
			priority, /* Priority at which the task is created. */
			nullptr
		);      
	}

	void print_time() {
		std::printf("time: %02x:%02x\n", mins, secs);
	}
}
