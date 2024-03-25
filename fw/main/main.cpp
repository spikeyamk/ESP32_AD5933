#include <trielo/trielo.hpp>

#include "util.hpp"
#include "i2c/bus.hpp"
#include "ble_server/server.hpp"
#include "auto_save_no_ble.hpp"
#include "sd_card.hpp"

inline void run() {
	while(I2C::Bus::get_instance().device_add(AD5933::SLAVE_ADDRESS) == false) {
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
	if(Util::mode_status == Util::Status::AutoSaveNoBLE) {
		Trielo::trielo<AutoSaveNoBLE::run>();
	} else {
		Util::Mode::get_instance();
		BLE::Server::inject(I2C::Bus::get_instance().active_device_handles.at(AD5933::SLAVE_ADDRESS));
	}
}

inline void test() {
	if(Trielo::trielo<SD_Card::init>(Trielo::OkErrCode(0)) != ESP_OK) {
		return;
	}
	if(Trielo::trielo<SD_Card::format>(Trielo::OkErrCode(0)) != ESP_OK) {
		return;
	}
	const std::string_view name { "test" };
	const auto start { std::chrono::high_resolution_clock::now() };
	if(Trielo::trielo<SD_Card::create_test_file>(Trielo::OkErrCode(0), (2 << 15), name) != 0) {
		return;
	}
	if(Trielo::trielo<SD_Card::check_test_file>(Trielo::OkErrCode(0), (2 << 15), name) != 0) {
		return;
	}
	const auto finish { std::chrono::high_resolution_clock::now() };
	std::cout << "std::chrono::duration_cast<std::chrono::milliseconds>(finish - start): " << std::chrono::duration_cast<std::chrono::milliseconds>(finish - start) << " ms" << std::endl;
}

extern "C" void app_main() {
	run();
}