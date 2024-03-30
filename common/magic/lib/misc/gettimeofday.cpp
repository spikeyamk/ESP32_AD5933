#include "magic/misc/gettimeofday.hpp"

#ifdef __linux__
	#include <chrono>
	mytimeval64_t mytimeval64_t::now() {
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
#elif _WIN32
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
	#undef WIN32_LEAN_AND_MEAN

	// https://gist.github.com/ugovaretto/5875385
	/* 
	* Author: Ugo Varetto - ugovaretto@gmail.com
	* This code is distributed under the terms of the Apache Software License version 2.0
	* https://opensource.org/licenses/Apache-2.0
	*/
	mytimeval64_t mytimeval64_t::now() {
		FILETIME ft;
		uint64_t tmpres = 0;
		static int tzflag = 0;
		GetSystemTimeAsFileTime(&ft);

		tmpres |= ft.dwHighDateTime;
		tmpres <<= 32;
		tmpres |= ft.dwLowDateTime;

		tmpres /= 10;  /*convert into microseconds*/
		/*converting file time to unix epoch*/
		#if defined(_MSC_VER) || defined(_MSC_EXTENSIONS)
			static constexpr uint64_t DELTA_EPOCH_IN_MICROSECS = 11644473600000000Ui64;
		#else
			static constexpr uint64_t DELTA_EPOCH_IN_MICROSECS = 11644473600000000ULL;
		#endif
		tmpres -= DELTA_EPOCH_IN_MICROSECS; 

		return mytimeval64_t {
			.tv_sec = static_cast<int64_t>(tmpres / 1000000UL),
			.tv_usec = static_cast<int32_t>(tmpres % 1000000UL),
		};
	}
#endif