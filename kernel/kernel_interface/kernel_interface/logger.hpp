#pragma once

namespace kernel_interface::logger {
	enum class LogLevel {
		Panic = 1,
		Error = 3,
		Warn = 4,
		Info = 6,
		Debug = 7,
	};

	class ILogger {
	public:
		virtual ~ILogger() = default;
		virtual void log(LogLevel level, const char* msg) = 0;

		virtual bool will_be_logged(LogLevel level) = 0;
	};

	inline ILogger* default_logger;
}
