#pragma once

#include <cstdio>

#include <kernel_interface/logger.hpp>

#include "graphics/console.hpp"

namespace logger {
	using namespace kernel_interface::logger;

	class ConsoleLogger final : public ILogger {
	public:
		ConsoleLogger(IConsole* console = nullptr, LogLevel log_level = LogLevel::Warn);

		void set_console(IConsole* console);

		void log(LogLevel level, const char* msg) override;
		bool will_be_logged(LogLevel level) override;

	private:
		IConsole* console_;
		LogLevel log_level_;
	};

	class LoggerProxy final {
	public:
		LoggerProxy(ILogger& logger);

		void log(LogLevel level, const char* msg);

		template <typename... Args>
		void error(const char* format, Args... args) {
			log_format(LogLevel::Error, format, args...);
		}

		template <typename... Args>
		void warn(const char* format, Args... args) {
			log_format(LogLevel::Warn, format, args...);
		}

		template <typename... Args>
		void info(const char* format, Args... args) {
			log_format(LogLevel::Info, format, args...);
		}

		template <typename... Args>
		void debug(const char* format, Args... args) {
			log_format(LogLevel::Debug, format, args...);
		}

	private:
		ILogger& logger_;

		template <typename... Args>
		void log_format(LogLevel level, const char* format, Args... args) {
			if (!logger_.will_be_logged(level)) {
				return;
			}

			char buf[1024];

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wformat-security"
			auto result = std::snprintf(buf, sizeof(buf), format, args...);
#pragma clang diagnostic pop

			if (result < 0) {
				return;
			}

			logger_.log(level, buf);
		}
	};
}

inline logger::LoggerProxy* log;
