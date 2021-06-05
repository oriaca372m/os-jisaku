#include <cerrno>
#include <sys/types.h>

extern "C" caddr_t sbrk(int incr) {
	return nullptr;
}

extern "C" void _exit() {
	while (true) {
		__asm__("hlt");
	}
}

extern "C" int getpid() {
	return 1;
}

extern "C" int kill(int pid, int sig) {
	errno = EINVAL;
	return -1;
}
