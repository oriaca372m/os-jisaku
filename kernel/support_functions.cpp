#include <cerrno>
#include <cstddef>
#include <new>
#include <sys/types.h>

#include "logger.hpp"
#include "utils.hpp"

[[noreturn]] void bad_call(const char* name) {
	log->error(u8"bad %s call!\n", name);
	halt();
}

extern "C" caddr_t sbrk(int incr) {
	bad_call("sbrk()");
}

extern "C" void _exit() {
	bad_call("exit()");
}

extern "C" int getpid() {
	bad_call("getpid()");
}

extern "C" int kill(int pid, int sig) {
	bad_call("kill()");
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
