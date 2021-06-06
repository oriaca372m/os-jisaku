#pragma once

#include <cstdio>

#include "console.hpp"

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

	global_console->put_string(buf);
	return result;
}

[[noreturn]] void halt();
