#include <thread>
#include <chrono>
#include <memory>

#include "ble.hpp"
#include "util.hpp"
#include "i2c.hpp"
#include "ds3231.hpp"

extern "C" void app_main() {
	Util::Blinky* blinky_instance = Util::Blinky::get_instance();
	blinky_instance->start(std::chrono::seconds(1));
	I2CBus i2c_bus {};
	i2c_bus.scan();
	auto ds3231 = run_ds3231(i2c_bus);
	std::this_thread::sleep_for(std::chrono::seconds(10));
	NimBLE::run();
}
