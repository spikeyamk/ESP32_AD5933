#include "util.hpp"
#include "ds3231.hpp"
#include "ble.hpp"

extern "C" void app_main() {
	static constinit const int blink_time_ms = 1000;
	Util::builtin_blinky(blink_time_ms);
	NimBLE::run();
}
