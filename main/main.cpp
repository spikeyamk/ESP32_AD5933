#include <thread>
#include <chrono>
#include <memory>
#include <atomic>
#include <vector>
#include <cstdint>
#include <algorithm>
#include <limits>
#include <array>
#include <future>

#include "freertos/FreeRTOS.h"
#include "freertos/FreeRTOSConfig.h"
#include "freertos/task.h"

#include "trielo/trielo.hpp"

#include "util.hpp"
#include "i2c/bus.hpp"
#include "ad5933/driver/driver.hpp"
#include "ad5933/driver/tests.hpp"
#include "magic/packets.hpp"
#include "ble/ble_server.hpp"
#include "sd_card.hpp"

extern "C" void app_main() {
	I2C::Bus i2c_bus {};
	i2c_bus.scan();
	while(i2c_bus.device_add(AD5933::SLAVE_ADDRESS) == false) {
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	AD5933::Driver ad5933_driver { i2c_bus.active_device_handles.at(AD5933::SLAVE_ADDRESS) };
	ad5933_driver.print_all_registers();
	Trielo::trielo<AD5933::Driver_Tests::runtime_test_driver>(Trielo::OkErrCode(0), ad5933_driver);
	BLE::Server::run();
	BLE::Server::advertise();
	//BLE::Server ble_server {};
	//ble_server.run();
	//ble_server.advertise();
}
