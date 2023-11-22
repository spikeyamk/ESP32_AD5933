#pragma once

#include <optional>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>

#include "led_strip.h"

namespace Util {
	void endless_debug_loop();
}

namespace Util {
	// Blinky Singleton
	class Blinky {
	private:
		// static pointer which points to the instance of this class
		static Blinky* instance_pointer; 
		static std::mutex mutex;

		std::atomic<bool> blink_task_enable = false;
		std::chrono::duration<int> blink_time;
		led_strip_handle_t led_strip_handle = nullptr;
		std::optional<std::thread> thread_handle = std::nullopt;

		// Making private Default constructor
		Blinky() {}

		// Deleting copy constructor
		Blinky(const Blinky& obj) = delete;
		// Deleting copy assignment operator
		void operator=(const Blinky& obj) = delete;

		static void blinky_cb(Blinky *self) {
			bool led_state = false;
			while(self->blink_task_enable == true || led_state == true) {
				self->toggle_led(led_state);
				/* Toggle the LED state */
				led_state = !led_state;
				std::this_thread::sleep_for(std::chrono::milliseconds(self->blink_time));
			}
		}
		friend void blinky_cb(Blinky *self);
	public:
		static Blinky* get_instance() {
    		std::lock_guard<std::mutex> lock(mutex);
			if(instance_pointer == nullptr) {
				instance_pointer = new Blinky(); 
			}
			return instance_pointer;
		}

		static bool destroy_instance() {
			if(instance_pointer == nullptr) {
				return false;
			} else {
				instance_pointer->stop();
				delete instance_pointer;
				return true;
			}
		}

		bool start(const std::chrono::duration<int> &in_blink_time) {
			if(blink_task_enable == true) {
				return false;
			}

			blink_time = in_blink_time;
			blink_task_enable = true;
			configure_led();
			std::thread tmp_thread(blinky_cb, this);
			thread_handle = std::move(tmp_thread);
			return true;
		}

		bool stop() {
			if(thread_handle.has_value() && thread_handle.value().joinable() && led_strip_handle != nullptr) {
				blink_task_enable = false;	
				thread_handle.value().join();
				led_strip_del(led_strip_handle);
				return true;
			} else {
				return false;
			}
		}
	private:
		void toggle_led(const bool led_state) {
			/* If the addressable LED is enabled */
			if(led_state == false) {
				/* Set the LED pixel using RGB from 0 (0%) to 255 (100%) for each color */
				led_strip_set_pixel(led_strip_handle, 0, 0, 10, 0);
				/* Refresh the strip to send data */
				led_strip_refresh(led_strip_handle);
			} else {
				/* Set all LED off to clear all pixels */
				led_strip_clear(led_strip_handle);
			}
		}

		void configure_led() {
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

			led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip_handle);
			// Set all LED off to clear all pixels
			led_strip_clear(led_strip_handle);
		}
	};
}


