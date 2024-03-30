#pragma once

#ifdef __riscv
	#include <sys/cdefs.h>
	#include <sys/time.h>
	using mytimeval64_t = timeval;
#else // for Unix and ESP-IDF
	#include <cstdint>
	struct mytimeval64_t {
		int64_t tv_sec;
		int32_t tv_usec;
		static mytimeval64_t now();
	};
#endif // end of ifdef _MSC_VER