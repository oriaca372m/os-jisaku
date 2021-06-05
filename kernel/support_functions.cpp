#include <cerrno>
#include <cstddef>
#include <new>
#include <sys/types.h>

#include "logger.hpp"

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

[[noreturn]] void bad_call(const char* name) {
	log->error(u8"bad %s call!\n", name);

	while (true) {
		__asm("hlt");
	}
}

void* operator new(std::size_t) {
	bad_call(u8"new(std::size_t)");
}

void operator delete(void*) noexcept {
	bad_call(u8"delete(void*)");
}
void operator delete(void*, std::align_val_t) noexcept {
	bad_call(u8"delete(void*, std::align_val_t)");
}

extern "C" void __cxa_pure_virtual() {
	bad_call(u8"__cxa_pure_virtual()");
}
