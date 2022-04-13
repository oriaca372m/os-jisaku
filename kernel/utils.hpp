#pragma once

#include <cstdio>

#include "graphics/console.hpp"
#include "timer.hpp"

template <typename... Args>
int printk(const char* format, Args... args) {
	char buf[1024];

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wformat-security"
	auto result = std::snprintf(buf, sizeof(buf), format, args...);
#pragma clang diagnostic pop

	if (result < 0) {
		return result;
	}

	start_lapic_timer();
	global_console->put_string(buf);
	const auto elapsed = lapic_timer_elapsed();
	stop_lapic_timer();

	std::snprintf(buf, sizeof(buf), "[%9d]", elapsed);
	global_console->put_string(buf);

	return result;
}

[[noreturn]] void halt();
