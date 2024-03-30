#pragma once

#ifdef __riscv
#include <sys/cdefs.h>
#include <sys/time.h>
using mytimeval64_t = timeval;
#else // for Unix and ESP-IDF
#include <cstdint>
#include <chrono>

struct mytimeval64_t {
	int64_t tv_sec;
	int32_t tv_usec;
	static mytimeval64_t now() {
		const auto now { std::chrono::high_resolution_clock::now() };
		const auto secs { std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()) };
		const auto usecs {
			std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch())
			- std::chrono::duration_cast<std::chrono::microseconds>(secs)
		};
		return mytimeval64_t {
			.tv_sec = secs.count(),
			.tv_usec = static_cast<int32_t>(usecs.count())
		};
	}
};
#endif // end of ifdef _MSC_VER