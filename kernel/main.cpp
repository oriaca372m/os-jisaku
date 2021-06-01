#include <cstdint>

#include "frame_buffer_config.hpp"

struct PixelColor {
	std::uint8_t r;
	std::uint8_t g;
	std::uint8_t b;
};

void WritePixel(const FrameBufferConfig& config, int x, int y, const PixelColor& c) {
	const int pixel_position = config.pixels_per_scan_line * y + x;

	auto p = config.frame_buffer + 4 * pixel_position;
	if (config.pixel_format == PixelFormat::RGBResv8BitPerColor) {
		p[0] = c.r;
		p[1] = c.g;
		p[2] = c.b;
	} else if (config.pixel_format == PixelFormat::BGRResv8BitPerColor) {
		p[0] = c.b;
		p[1] = c.g;
		p[2] = c.r;
	}
}

extern "C" void KernelMain(const FrameBufferConfig& frame_buffer_config) {
	for (int x = 0; x < frame_buffer_config.horizontal_resolution; ++x) {
		for (int y = 0; y < frame_buffer_config.vertical_resolution; ++y) {
			WritePixel(frame_buffer_config, x, y, {0xFF, 0xFF, 0xFF});
		}
	}

	for (int x = 0; x < 200; ++x) {
		for (int y = 0; y < 100; ++y) {
			WritePixel(frame_buffer_config, x, y, {0x0, 0xFF, 0x0});
		}
	}

	while (true) {
		__asm("hlt");
	}
}
