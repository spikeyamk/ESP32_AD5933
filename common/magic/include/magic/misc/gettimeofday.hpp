#pragma once

#ifdef __riscv
	#include <sys/cdefs.h>
	#include <sys/time.h>
	using mytimeval64_t = timeval;
#else // for Unix and ESP-IDF
	#ifdef __linux__
		#include <cstdint>
		#include <chrono>
	#elif _WIN32
		#include <boost/date_time/posix_time/posix_time.hpp>
	#endif

	struct mytimeval64_t {
		int64_t tv_sec;
		int32_t tv_usec;
		#ifdef __linux__
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
		#elif _WIN32
			static mytimeval64_t now() {
				const auto now { boost::posix_time::microsec_clock::universal_time() };
				return mytimeval64_t {
					.tv_sec = boost::posix_time::to_time_t(now),
					.tv_usec = static_cast<int32_t>(now.time_of_day().fractional_seconds()),
				};
			}
		#endif
	};
#endif // end of ifdef _MSC_VER