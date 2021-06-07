#pragma once

#include <cstdint>

enum class PixelFormat {
	RGBResv8BitPerColor,
	BGRResv8BitPerColor,
};

struct FrameBufferConfig {
	std::uint8_t* frame_buffer;
	std::uint32_t pixels_per_scan_line;
	std::uint32_t horizontal_resolution;
	std::uint32_t vertical_resolution;
	PixelFormat pixel_format;
};
