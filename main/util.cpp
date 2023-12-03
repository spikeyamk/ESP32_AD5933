#include <thread>
#include <chrono>
#include <vector>
#include <cstdint>
#include <iostream>
#include <algorithm>
#include "trielo/trielo.hpp"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cstdint>
#include <numeric>

#include "ad5933.hpp"


#include "util.hpp"
#include "esp_task_wdt.h"

namespace Util {
	void endless_debug_loop() {
		while(1) {
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}
	}

	std::mutex Blinky::mutex;
}


namespace Util {
	namespace Benchmarks {
		void calculate_primes_cb(void *arg) {
			const uint32_t stopper = *static_cast<uint32_t*>(arg);
			const auto start = std::chrono::high_resolution_clock::now();
			auto end = start;
			for(uint32_t number = 2; number < stopper; ) {
				const auto chunk_start = std::chrono::high_resolution_clock::now();
				std::vector<uint32_t> primes;
				primes.clear();
				size_t reserve_bytes = 4096;
				try {
					primes.reserve(reserve_bytes);
				} catch(...) {
					std::cerr << "ERROR: prime_test(" << stopper << "): failed to primes.reserve()\n";
					reserve_bytes--;
				}
				size_t index = 0;
				for(; number < stopper; number++) {
					uint32_t divisor = (number - 1);
					for(; divisor > 1; divisor--) {
						if((number % divisor) == 0) {
							break;
						}
					}
					if(divisor == 1) {
						try {
							primes.push_back(number);
						} catch(...) {
							std::cerr << "ERROR: prime_test(" << stopper << "): failed to primes.push_back(number)\n";
							break;
						}
					}
				}
				const auto chunk_end = std::chrono::high_resolution_clock::now();
				end += (chunk_end - chunk_start);

				std::find_if(primes.begin(), primes.end(), [print_index = 0, index](const uint32_t element) mutable {
					if(index == 0 || element != 0) {
						std::printf("primes[%zu]: %lu\n", print_index++, element);
						return false;
					} else {
						return true;
					}
				});

				std::cout << "prime_test(): calcation chunk duration: " << chunk_end - chunk_start << std::endl;
			}
			std::cout << "prime_test(): " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start) << std::endl;
			vTaskDelete(NULL);
		}
		
		void calculate_primes_endless_test() {
			if(Trielo::trielo<esp_task_wdt_deinit>(Trielo::OkErrCode(ESP_OK)) == ESP_OK) {
				while(1) {
					uint32_t primes_stopper = 30'000u;
					TaskHandle_t calculate_primes_task_handle;
					xTaskCreate(
						calculate_primes_cb,
						"calculate_primes_cb",
						2048,
						static_cast<void*>(&primes_stopper),
						configMAX_PRIORITIES,
						&calculate_primes_task_handle
					);

					TaskStatus_t taskStatus;
					do {
						vTaskGetInfo(calculate_primes_task_handle, &taskStatus, pdTRUE, eInvalid);
						std::this_thread::sleep_for(std::chrono::milliseconds(100));
					} while(taskStatus.eCurrentState != eDeleted);
					std::this_thread::sleep_for(std::chrono::seconds(10));
				}
			}
		}

		static std::chrono::microseconds time_dump_all_registers_as_array() {
			const auto start = std::chrono::high_resolution_clock::now();
			const auto ret_registers { AD5933_Tests::ad5933.load()->dump_all_registers_as_array() };
			if(ret_registers.has_value() == false) {
				return std::chrono::duration_cast<std::chrono::microseconds>(start - start);
			}
			const auto end = std::chrono::high_resolution_clock::now();

			// Disabled debugging prints
			//std::for_each(ret_registers.value().begin(), ret_registers.value().end(), [print_index = AD5933_Tests::ad5933.load()->lowest_register_address.unwrap()](const uint8_t e) mutable {
			//	std::printf("Register[0x%02X]: 0x%02X\n", print_index++, e); 
			//});
			return std::chrono::duration_cast<std::chrono::microseconds>(end - start);
		}

		static void time_dump_all_registers_cb(void *arg) {
			std::vector<std::chrono::microseconds> duration_rets;
			const size_t num_of_iterations = *reinterpret_cast<size_t*>(arg);
			for(size_t i = 0; i < num_of_iterations; i++) {
				duration_rets.push_back(time_dump_all_registers_as_array());
			}

			const std::chrono::microseconds duration_sum { std::accumulate(duration_rets.begin(), duration_rets.end(), std::chrono::microseconds(0)) };
			const std::chrono::microseconds duration_average { duration_sum / duration_rets.size() };

			std::sort(duration_rets.begin(), duration_rets.end(), [](const auto& a, const auto& b) {
    			return a.count() < b.count();
			}); // This weird sort was sorting downwards that's why I added the lambda

			const std::chrono::microseconds duration_median { [&duration_rets]() -> std::chrono::microseconds {
				const size_t middle = duration_rets.size() / 2;
				if((duration_rets.size() % 2) == 0) {
					return (duration_rets[middle - 1] + duration_rets[middle]) / 2;
				} else {
					return duration_rets[middle];
				}
			}()};

			const std::chrono::microseconds duration_min { duration_rets.front() };
			const std::chrono::microseconds duration_max { duration_rets.back() };

			std::cout << "time_dump_all_registers_cb(): \n";
			std::cout << "\tduration_sum:     " << duration_sum << std::endl;
			std::cout << "\tduration_average: " << duration_average << std::endl;
			std::cout << "\tduration_median:  " << duration_median << std::endl;
			std::cout << "\tduration_min:     " << duration_min << std::endl;
			std::cout << "\tduration_max:     " << duration_max << std::endl;
			vTaskDelete(nullptr);
		}

		void create_dump_all_registers_task(size_t iterations) {
			if(Trielo::trielo<esp_task_wdt_deinit>(Trielo::OkErrCode(ESP_OK)) == ESP_OK) {
				xTaskCreate(
					time_dump_all_registers_cb,
					"dump_all",
					2048,
					static_cast<void*>(&iterations),
					configMAX_PRIORITIES,
					nullptr
				);
			}
		}
	}

	void print_running_tasks() {
		TaskStatus_t *taskList;
		uint32_t totalTasks;

		// Get the number of tasks in the system
		totalTasks = uxTaskGetNumberOfTasks();

		// Allocate memory for the task list
		taskList = (TaskStatus_t*)pvPortMalloc(sizeof(TaskStatus_t) * totalTasks);

		if (taskList != nullptr) {
			// Get information about each task
			totalTasks = uxTaskGetSystemState(taskList, totalTasks, nullptr);

			// Print information about each task
			for (uint32_t i = 0; i < totalTasks; i++) {
				printf("Task name: %s, Task priority: %u, Task state: %s\n",
					taskList[i].pcTaskName,
					taskList[i].uxCurrentPriority,
					(taskList[i].eCurrentState == eRunning) ? "Running" : "Not Running");
			}

			// Free the allocated memory for the task list
			vPortFree(taskList);
		}
	}

	std::string uint8ToHexString(uint8_t value) {
		std::stringstream ss;
		ss << "0x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(value);
		return ss.str();
	}

}


