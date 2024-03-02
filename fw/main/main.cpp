#include <trielo/trielo.hpp>

#include "util.hpp"
#include "i2c/bus.hpp"
#include "sd_card.hpp"
#include "ble/server/server.hpp"
#include "magic/events/results.hpp"

extern "C" void app_main() {
	Trielo::trielo<Util::init_restart_button>();
	Trielo::trieloxit<SD_Card::init>(Trielo::OkErrCode(0));
	I2C::Bus i2c_bus {};
	i2c_bus.scan();
	while(i2c_bus.device_add(AD5933::SLAVE_ADDRESS) == false) {
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
	BLE::Server::inject(i2c_bus.active_device_handles.at(AD5933::SLAVE_ADDRESS));
	Util::endless_debug_loop();
}