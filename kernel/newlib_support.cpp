#include <sys/types.h>

extern "C" caddr_t sbrk(int incr) {
	return nullptr;
}
