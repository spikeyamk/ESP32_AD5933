#pragma once

#ifdef _MSC_VER // for stupid Windows
// https://gist.github.com/ugovaretto/5875385
/* 
* Author: Ugo Varetto - ugovaretto@gmail.com
* This code is distributed under the terms of the Apache Software License version 2.0
* https://opensource.org/licenses/Apache-2.0
*/
#include <cstdint>
#include <time.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN

#if defined(_MSC_VER) || defined(_MSC_EXTENSIONS)
static constexpr uint64_t DELTA_EPOCH_IN_MICROSECS = 11644473600000000Ui64;
#else
static constexpr uint64_t DELTA_EPOCH_IN_MICROSECS = 11644473600000000ULL;
#endif

struct timezone {
	int tz_minuteswest;
	int tz_dsttime;
};

struct timeval {
	int64_t tv_sec;
	int64_t tv_usec;
};

int gettimeofday(timeval *tv, timezone *tz);
#else // for Unix and ESP-IDF
#include <sys/cdefs.h>
#include <sys/time.h>
#endif // end of ifdef _MSC_VER