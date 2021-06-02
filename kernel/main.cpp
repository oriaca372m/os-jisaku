#include <cstddef>
#include <cstdint>
#include <cstdio>

#include "console.hpp"
#include "font.hpp"
#include "frame_buffer_config.hpp"
#include "graphics.hpp"

void* operator new(std::size_t, void* buf) {
	return buf;
}

void operator delete(void*) noexcept {}

extern "C" void KernelMain(const FrameBufferConfig& frame_buffer_config) {
	char pixel_writer_buf[sizeof(RGBResv8BitPerColorPixelWriter)];
	PixelWriter* pixel_writer = reinterpret_cast<PixelWriter*>(pixel_writer_buf);

	if (frame_buffer_config.pixel_format == PixelFormat::RGBResv8BitPerColor) {
		new (pixel_writer_buf) RGBResv8BitPerColorPixelWriter(frame_buffer_config);
	} else if (frame_buffer_config.pixel_format == PixelFormat::BGRResv8BitPerColor) {
		new (pixel_writer_buf) BGRResv8BitPerColorPixelWriter(frame_buffer_config);
	}

	Console console(*pixel_writer, {0xc8, 0xc8, 0xc6}, {0x1d, 0x1f, 0x21});

	console.put_string(u8"chino chan kawaii!\n");
	console.put_string(u8"gochuumon wa usagi desu ka?\n");

	for (int i = 0; i < 100; i++) {
		char buf[128];
		std::snprintf(buf, sizeof(buf), "i = %d", i);
		console.put_string(buf);
		console.put_string(u8"chino chan kawaii!");

		for (volatile int j = 0; j < 100000000; j++) {
		}
	}

	while (true) {
		__asm("hlt");
	}
}
