#include "logger.hpp"

#include <cstddef>
#include <cstdio>

#include <kernel_interface/logger.hpp>

namespace usb::logger {
	int Log(LogLevel level, const char* format, ...) {
		using KLogLevel = kernel_interface::logger::LogLevel;
		KLogLevel k_log_level;

		switch (level) {
		case LogLevel::kError:
			k_log_level = KLogLevel::Error;
			break;
		case LogLevel::kWarn:
			k_log_level = KLogLevel::Warn;
			break;
		case LogLevel::kInfo:
			k_log_level = KLogLevel::Info;
			break;
		case LogLevel::kDebug:
			k_log_level = KLogLevel::Debug;
			break;
		default:
			return 1;
		}

		if (!kernel_interface::logger::default_logger->will_be_logged(k_log_level)) {
			return 0;
		}

		va_list ap;
		int result;
		char s[1024];

		va_start(ap, format);
		result = vsprintf(s, format, ap);
		va_end(ap);

		kernel_interface::logger::default_logger->log(k_log_level, s);
		return result;
	}
}
