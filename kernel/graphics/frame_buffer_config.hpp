#pragma once

#include <cstddef>
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

	FrameBufferConfig(std::uint32_t width, std::uint32_t height, PixelFormat pixel_format) :
		frame_buffer(nullptr),
		pixels_per_scan_line(width),
		horizontal_resolution(width),
		vertical_resolution(height),
		pixel_format(pixel_format) {}
};
