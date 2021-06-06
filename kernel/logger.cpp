#include "logger.hpp"

namespace logger {
	ConsoleLogger::ConsoleLogger(Console& console, LogLevel log_level) : console_{console}, log_level_{log_level} {}

	void ConsoleLogger::log(LogLevel level, const char* msg) {
		if (will_be_logged(level)) {
			console_.put_string(msg);
		}
	}

	bool ConsoleLogger::will_be_logged(LogLevel level) {
		return level <= log_level_;
	}

	LoggerProxy::LoggerProxy(ILogger& logger) : logger_{logger} {}

	void LoggerProxy::log(LogLevel level, const char* msg) {
		logger_.log(level, msg);
	}
}