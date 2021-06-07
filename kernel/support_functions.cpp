#include "logger.hpp"
#include "utils.hpp"

[[noreturn]] void bad_call(const char* name) {
	log->error(u8"bad %s call!\n", name);
	halt();
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

extern "C" void __cxa_pure_virtual() {
	bad_call(u8"__cxa_pure_virtual()");
}

extern "C" int posix_memalign(void**, std::size_t, std::size_t) {
	bad_call(u8"posix_memalign()");
}

extern "C" void close() {
	bad_call(u8"close()");
}

extern "C" void read() {
	bad_call(u8"read()");
}

extern "C" void lseek() {
	bad_call(u8"lseek()");
}

extern "C" void write() {
	bad_call(u8"write()");
}

extern "C" void fstat() {
	bad_call(u8"fstat()");
}

extern "C" void isatty() {
	bad_call(u8"isatty()");
}
