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
#include "host/ble_gap.h"

#include "trielo/trielo.hpp"

#include "util.hpp"
#include "i2c/bus.hpp"

#include "sd_card.hpp"

#include "ble/server/server.hpp"

extern "C" void app_main() {
	Trielo::trielo<Util::restart_button>();
	Trielo::trieloxit<SD_Card::init>(Trielo::OkErrCode(0));
	Trielo::trielo<SD_Card::create_test_files>();
	Trielo::trielo<SD_Card::print_test_files>();
	I2C::Bus i2c_bus {};
	while(i2c_bus.device_add(AD5933::SLAVE_ADDRESS) == false) {
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	BLE::Server::inject(i2c_bus.active_device_handles.at(AD5933::SLAVE_ADDRESS));
	while(1) {
		std::this_thread::sleep_for(std::chrono::seconds(10));
	}
	Trielo::trielo<SD_Card::deinit>();
}
