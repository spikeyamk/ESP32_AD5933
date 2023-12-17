#pragma once

#include <optional>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>
#include <string>
#include <array>
#include <utility>

#include "led_strip.h"

#include "ad5933/driver/driver.hpp"

namespace Util {
	void endless_debug_loop();
}

namespace Util {
	// Blinky Singleton
	class Blinky {
	private:
		// static pointer which points to the instance of this class

		static std::mutex mutex;
		std::atomic<bool> blink_task_enable = false;
		std::chrono::duration<int, std::milli> blink_time;
		led_strip_handle_t led_strip_handle = nullptr;
		std::optional<std::thread> thread_handle = std::nullopt;

		// Making private Default constructor
		Blinky();

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
		static Blinky& get_instance() {
    		std::unique_lock lock(mutex);
        	static Blinky instance; // Static instance created once
			return instance;
		}
		bool start(const std::chrono::duration<int, std::milli> &in_blink_time);
		bool stop();
		void set_blink_time(const std::chrono::duration<int, std::milli> &in_blink_time);
		bool is_running() const;
	private:
		void toggle_led(const bool led_state);
		void configure_led();
	};
}

namespace Util {
	namespace Benchmarks {
		void calculate_primes_cb(void *arg);
		void calculate_primes_endless_test();
		void create_dump_all_registers_task(const size_t iterations, const AD5933::Driver &driver);
	}
	void print_running_tasks();
	std::string uint8ToHexString(uint8_t value);

	template <typename T, std::size_t N1, std::size_t N2>
	constexpr std::array<T, N1 + N2> concat_diff_size_arrays(std::array<T, N1> lhs, std::array<T, N2> rhs) {
		std::array<T, N1 + N2> result{};
		std::size_t index = 0;

		for (auto& el : lhs) {
			result[index] = std::move(el);
			++index;
		}
		for (auto& el : rhs) {
			result[index] = std::move(el);
			++index;
		}

		return result;
	}

	void restart_button();
}