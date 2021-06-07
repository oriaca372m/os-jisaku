#pragma once

#include <cstdint>

#include "x86_descriptor.hpp"

union SegmentDescriptor {
	std::uint64_t data;
	struct {
		std::uint16_t limit_low : 16;
		std::uint16_t base_low : 16;
		std::uint8_t base_middle : 8;
		DescriptorType type : 4;
		std::uint8_t system_segment : 1;
		std::uint8_t descriptor_privilege_level : 2;
		std::uint8_t present : 1;
		std::uint8_t limit_high : 4;
		std::uint8_t available : 1;
		std::uint8_t long_mode : 1;
		std::uint8_t default_operation_size : 1;
		std::uint8_t granularity : 1;
		std::uint8_t base_high : 8;
	} __attribute__((packed)) bits;
} __attribute__((packed));

void setup_segments();
