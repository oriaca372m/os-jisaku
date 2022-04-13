#include "timer.hpp"

#include <limits>

namespace {
	const std::uint32_t count_max = std::numeric_limits<std::uint32_t>::max();
	volatile std::uint32_t& lvt_timer = *reinterpret_cast<std::uint32_t*>(0xfee00320);
	volatile std::uint32_t& initial_count = *reinterpret_cast<std::uint32_t*>(0xfee00380);
	volatile std::uint32_t& current_count = *reinterpret_cast<std::uint32_t*>(0xfee00390);
	volatile std::uint32_t& divide_config = *reinterpret_cast<std::uint32_t*>(0xfee003e0);
}

void initialize_lapic_timer() {
	divide_config = 0b1011;
	lvt_timer = (0b001 << 16) | 0b10'0000;
}

void start_lapic_timer() {
	initial_count = count_max;
}

std::uint32_t lapic_timer_elapsed() {
	return count_max - current_count;
}

void stop_lapic_timer() {
	initial_count = 0;
}
