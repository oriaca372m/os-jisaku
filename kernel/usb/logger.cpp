#include "logger.hpp"

#include <cstddef>
#include <cstdio>

#include <kernel_interface/main.hpp>

namespace {
	LogLevel log_level = kDebug;
}

namespace usb::logger {
	void SetLogLevel(LogLevel level) {
		log_level = level;
	}

	int Log(LogLevel level, const char* format, ...) {
		if (level > log_level) {
			return 0;
		}

		va_list ap;
		int result;
		char s[1024];

		va_start(ap, format);
		result = vsprintf(s, format, ap);
		va_end(ap);

		kernel_interface::put_string(s);
		return result;
	}
}
