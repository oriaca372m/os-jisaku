#include "sbrk.hpp"

#include <cerrno>
#include <sys/types.h>

namespace {
	caddr_t program_break;
	caddr_t program_break_end;
}

extern "C" caddr_t sbrk(int incr) {
	if (program_break == 0 || program_break + incr >= program_break_end) {
		errno = ENOMEM;
		return reinterpret_cast<caddr_t>(-1);
	}

	caddr_t prev_break = program_break;
	program_break += incr;
	return prev_break;
}

Error initialize_heap(BitmapMemoryManager& memory_manager) {
	constexpr int heap_frames = 64 * 512;
	const auto heap_start = memory_manager.allocate(heap_frames);
	if (heap_start.error) {
		return heap_start.error;
	}

	program_break = reinterpret_cast<caddr_t>(heap_start.value.id() * bytes_per_frame);
	program_break_end = program_break + heap_frames * bytes_per_frame;
	return Error::Code::Success;
}
