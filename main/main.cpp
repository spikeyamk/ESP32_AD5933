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
#include <functional>
#include <span>

#include "freertos/FreeRTOS.h"
#include "freertos/FreeRTOSConfig.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "trielo/trielo.hpp"

#include "util.hpp"
#include "i2c/bus.hpp"
#include "ad5933/driver/driver.hpp"
#include "ad5933/extension/extension.hpp"
//#include "ble/sender/sender.hpp"
//#include "ble/server/server.hpp"
//#include "ble/state_machine/state_machine.hpp"
#include "sd_card.hpp"

extern "C" void app_main() {
	I2C::Bus i2c_bus {};
	while(i2c_bus.device_add(AD5933::SLAVE_ADDRESS) == false) {
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	AD5933::Driver ad5933_driver { i2c_bus.active_device_handles.at(AD5933::SLAVE_ADDRESS) };
	AD5933::Extension ad5933_extension { ad5933_driver };
	//BLE::Sender sender {};
	//std::atomic<bool> processing = false;
	//BLE::T_StateMachine ble_state_machine { sender, ad5933_extension, processing };
	//BLE::Server::run();
}
