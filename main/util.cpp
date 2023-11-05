#include <thread>
#include <chrono>

#include "util.hpp"

namespace Util {
	void endless_debug_loop() {
		while(1) {
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}
	}

	Blinky* Blinky::instance_pointer = nullptr;
	std::mutex Blinky::mutex;
}
