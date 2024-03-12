#pragma once

#ifdef _MSC_VER // for stupid Windows
// https://gist.github.com/ugovaretto/5875385
/* 
* Author: Ugo Varetto - ugovaretto@gmail.com
* This code is distributed under the terms of the Apache Software License version 2.0
* https://opensource.org/licenses/Apache-2.0
*/
#include <cstdint>

struct timezone {
	int tz_minuteswest;
	int tz_dsttime;
};

struct mytimeval64_t {
	int64_t tv_sec;
	int64_t tv_usec;
};

int gettimeofday(mytimeval64_t *tv, timezone *tz);
#else // for Unix and ESP-IDF
#include <sys/cdefs.h>
#include <sys/time.h>
using mytimeval64_t = timeval;
#endif // end of ifdef _MSC_VER