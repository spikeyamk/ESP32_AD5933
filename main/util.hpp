#pragma once

#include <cstdint>

namespace Util {
	void endless_debug_loop();
	void sleep_ms(const uint32_t delay);
	void builtin_blinky(const uint32_t delay);
}
