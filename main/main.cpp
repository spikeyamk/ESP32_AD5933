#include <chrono>

#include "ble.hpp"
#include "util.hpp"

extern "C" void app_main() {
	Util::Blinky* blinky_instance = Util::Blinky::get_instance();
	blinky_instance->start(std::chrono::seconds(1));
	NimBLE::run();
}
