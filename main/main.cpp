#include <thread>
#include <chrono>
#include <memory>
#include <atomic>
#include <vector>
#include <cstdint>
#include <algorithm>
#include <limits>
#include <array>
#include "trielo/trielo.hpp"
#include <future>

#include "util.hpp"
#include "i2c.hpp"
#include "ad5933.hpp"
#include "ble_state_machine/include/ble.hpp"
#include "ble_state_machine/include/ble_state_machine.hpp"
#include "ble_state_machine/include/ble_states.hpp"
#include "sd_card.hpp"

#include "freertos/FreeRTOS.h"
#include "freertos/FreeRTOSConfig.h"
#include "freertos/task.h"

extern "C" void app_main() {
	Util::Blinky& blinky_instance = Util::Blinky::get_instance();
	blinky_instance.start(std::chrono::seconds(1));
	I2CBus i2c_bus {};
	i2c_bus.scan();
	AD5933_Tests::init_ad5933(i2c_bus);

	state_machine.turn_on();
	state_machine.change_to_state(static_cast<const States::bState*>(&States::advertising));

	while(1) {
		std::this_thread::sleep_for(std::chrono::seconds(100));
	}
}
