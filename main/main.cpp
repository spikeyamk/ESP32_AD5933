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

#include "freertos/FreeRTOS.h"
#include "freertos/FreeRTOSConfig.h"
#include "freertos/task.h"

#include "trielo/trielo.hpp"

#include "util.hpp"
#include "i2c/bus.hpp"
#include "ad5933/driver/driver.hpp"
#include "ad5933/driver/tests.hpp"
#include "ad5933/extension/extension.hpp"
#include "magic/packets.hpp"
#include "ble/server/server.hpp"
#include "ble/sender/sender.hpp"
#include "sd_card.hpp"

#define TRIELO_EQ(call, expected) do {\
    const auto result = call;\
	if(result == expected) {\
    	fmt::print(fmt::fg(fmt::color::green), "Success: ");\
	} else {\
		fmt::print(fmt::fg(fmt::color::red), "ERROR: ");\
	}\
    std::cout << #call << ": '" << result << "'\n";\
} while (false)

#define TRIELO(call) std::cout << #call << ": '" << call << "'\n";

extern "C" void app_main() {
	I2C::Bus i2c_bus {};
	i2c_bus.scan();
	while(i2c_bus.device_add(AD5933::SLAVE_ADDRESS) == false) {
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	AD5933::Driver ad5933_driver { i2c_bus.active_device_handles.at(AD5933::SLAVE_ADDRESS) };
	ad5933_driver.print_all_registers();
	Trielo::trielo<AD5933::Driver_Tests::runtime_test_driver>(Trielo::OkErrCode(0), ad5933_driver);
	AD5933::Extension ad5933_extension { ad5933_driver };
	TRIELO_EQ(ad5933_extension.has_status_condition(AD5933::Masks::Or::Status::FreqSweepComplete), true);
	TRIELO(ad5933_extension.has_status_condition(AD5933::Masks::Or::Status::FreqSweepComplete));
	BLE::Server::run();
	BLE::Server::advertise();
	BLE::Sender ble_sender {};
	ble_sender.send(std::array<uint8_t, 20> { 0x00, 0x01, 0x02 });
}
