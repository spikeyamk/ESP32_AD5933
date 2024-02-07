#pragma once

#ifdef _MSC_VER // for stupid Windows
// https://gist.github.com/ugovaretto/5875385
/* 
* Author: Ugo Varetto - ugovaretto@gmail.com
* This code is distributed under the terms of the Apache Software License version 2.0
* https://opensource.org/licenses/Apache-2.0
*/
#include "magic/misc/gettimeofday.hpp"

int gettimeofday(timeval *tv, timezone *tz) {
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
		tmpres -= DELTA_EPOCH_IN_MICROSECS; 
		tv->tv_sec = static_cast<int64_t>(tmpres / 1000000UL);
		tv->tv_usec = static_cast<int64_t>(tmpres % 1000000UL);
	}

	if(tz != nullptr) {
		if(tzflag == 0) {
			_tzset();
			tzflag++;
		}
		tz->tz_minuteswest = _timezone / 60;
		tz->tz_dsttime = _daylight;
	}
	return 0;
}
#endif // end of ifdef _MSC_VER