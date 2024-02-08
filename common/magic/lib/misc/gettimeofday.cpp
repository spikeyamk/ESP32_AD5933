#pragma once

#ifdef _MSC_VER // for stupid Windows
// https://gist.github.com/ugovaretto/5875385
/* 
* Author: Ugo Varetto - ugovaretto@gmail.com
* This code is distributed under the terms of the Apache Software License version 2.0
* https://opensource.org/licenses/Apache-2.0
*/
#include <time.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN

#include "magic/misc/gettimeofday.hpp"

int gettimeofday(mytimeval64_t *tv, timezone *tz) {
	FILETIME ft;
	uint64_t tmpres = 0;
	static int tzflag = 0;

	if(tv != nullptr) {
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
		tv->tv_sec = static_cast<int64_t>(tmpres / 1000000UL);
		tv->tv_usec = static_cast<int64_t>(tmpres % 1000000UL);
	}

	if(tz != nullptr) {
		if(tzflag == 0) {
			_tzset();
			tzflag++;
		}
		#define _CRT_SECURE_NO_WARNINGS
		tz->tz_minuteswest = _timezone / 60;
		tz->tz_dsttime = _daylight;
		#undef _CRT_SECURE_NO_WARNINGS
	}
	return 0;
}
#endif // end of ifdef _MSC_VER