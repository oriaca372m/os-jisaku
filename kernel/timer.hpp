#pragma once

#include <cstdint>

void initialize_lapic_timer();
void start_lapic_timer();
std::uint32_t lapic_timer_elapsed();
void stop_lapic_timer();
