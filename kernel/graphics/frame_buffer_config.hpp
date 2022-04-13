#pragma once

#include <cstddef>
#include <cstdint>

enum class PixelFormat {
	RGBResv8BitPerColor,
	BGRResv8BitPerColor,
};

inline std::size_t bytes_per_pixel(PixelFormat pixel_format) {
	switch (pixel_format) {
	case PixelFormat::RGBResv8BitPerColor:
		return 4;
	case PixelFormat::BGRResv8BitPerColor:
		return 4;
	}
}

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
