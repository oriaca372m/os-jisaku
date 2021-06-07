#include <kernel_interface/main.hpp>

#include "graphics/console.hpp"

namespace kernel_interface {
	int put_string(const char* str) {
		global_console->put_string(str);
		return 0;
	}
}
