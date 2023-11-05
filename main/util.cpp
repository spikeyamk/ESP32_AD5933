/* Blink Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led_strip.h"

#include "util.hpp"

namespace Util {
	void sleep_ms(const uint32_t delay) {
		const TickType_t xDelay = delay / portTICK_PERIOD_MS;
		vTaskDelay(xDelay);
	}

	void endless_debug_loop() {
		while(1) {
			sleep_ms(100);
		}
	}
}

namespace Util {
	static led_strip_handle_t led_strip;

	static void blink_led(const bool led_state) {
		/* If the addressable LED is enabled */
		if(led_state == false) {
			/* Set the LED pixel using RGB from 0 (0%) to 255 (100%) for each color */
			led_strip_set_pixel(led_strip, 0, 0, 10, 0);
			/* Refresh the strip to send data */
			led_strip_refresh(led_strip);
		} else {
			/* Set all LED off to clear all pixels */
			led_strip_clear(led_strip);
		}
	}

	static void configure_led() {
		/* LED strip initialization with the GPIO and pixels number*/
		led_strip_config_t strip_config = {
			.strip_gpio_num = 8,
			.max_leds = 1, // at least one LED on board
			.led_pixel_format = LED_PIXEL_FORMAT_GRB,
			.led_model = LED_MODEL_WS2812,
			.flags {
				.invert_out = 0
			}
		};

		led_strip_rmt_config_t rmt_config = {
			.clk_src = RMT_CLK_SRC_DEFAULT,
			.resolution_hz = 10 * 1000 * 1000, // 10MHz
			.mem_block_symbols = 0,
			.flags {
				.with_dma = false
			}
		};

    	led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip);
    	// Set all LED off to clear all pixels
    	led_strip_clear(led_strip);
	}

	static void builtin_blinky_cb(void *pvParameters) {
		//configASSERT(std::reinterpret_cast<uint32_t>(pvParameters) >= 500);
		/* Configure the peripheral according to the LED type */
		configure_led();

		bool led_state = false;
		while(1) {
			blink_led(led_state);
			/* Toggle the LED state */
			led_state = !led_state;
			sleep_ms((uint32_t) pvParameters);
		}
	}
}

namespace Util {
	/* Function that creates a task. */
	void builtin_blinky(const uint32_t delay) {
		const uint32_t STACK_SIZE = 2*1024;
		const uint32_t priority = 1;

		xTaskCreate(
			builtin_blinky_cb,   /* Function that implements the task. */
			"builtin_blinky", /* Text name for the task. */
			STACK_SIZE,       /* Stack size in words, not bytes. */
			(void*) delay, /* Parameter passed into the task. */
			priority, /* Priority at which the task is created. */
			nullptr /* Used to pass out the created task's handle. */
		);      
	}
}

