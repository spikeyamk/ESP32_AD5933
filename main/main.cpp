#include <thread>
#include <chrono>
#include <memory>
#include <atomic>

#include "util.hpp"
#include "i2c.hpp"
#include "ds3231.hpp"
#include "ad5933.hpp"
#include "ble.hpp"

extern "C" void app_main() {
	Util::Blinky* blinky_instance = Util::Blinky::get_instance();
	blinky_instance->start(std::chrono::seconds(1));

	I2CBus i2c_bus {};
	//i2c_bus.scan();
	std::this_thread::sleep_for(std::chrono::seconds());

	/*
		auto ds3231 = run_ds3231(i2c_bus);
		std::this_thread::sleep_for(std::chrono::seconds(1));
	*/
	AD5933_Tests::init_ad5933(i2c_bus);
	AD5933_Tests::impedance_try();

	// NimBLE::run();
	while(1) {
		std::this_thread::sleep_for(std::chrono::seconds(1000));
	}
}
