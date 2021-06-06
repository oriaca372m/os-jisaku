#include "utils.hpp"

void halt() {
	while (true) {
		__asm("hlt");
	}
}
