#include <cstddef>
#include <cstdint>

#include "font.hpp"
#include "frame_buffer_config.hpp"
#include "graphics.hpp"

void* operator new(std::size_t, void* buf) {
	return buf;
}

void operator delete(void*) noexcept {}

extern "C" void __cxa_pure_virtual() {
	while (true) {
		__asm("hlt");
	}
}

extern "C" void KernelMain(const FrameBufferConfig& frame_buffer_config) {
	char pixel_writer_buf[sizeof(RGBResv8BitPerColorPixelWriter)];
	PixelWriter* pixel_writer = reinterpret_cast<PixelWriter*>(pixel_writer_buf);

	if (frame_buffer_config.pixel_format == PixelFormat::RGBResv8BitPerColor) {
		new (pixel_writer_buf) RGBResv8BitPerColorPixelWriter(frame_buffer_config);
	} else if (frame_buffer_config.pixel_format == PixelFormat::BGRResv8BitPerColor) {
		new (pixel_writer_buf) BGRResv8BitPerColorPixelWriter(frame_buffer_config);
	}

	for (int x = 0; x < frame_buffer_config.horizontal_resolution; ++x) {
		for (int y = 0; y < frame_buffer_config.vertical_resolution; ++y) {
			pixel_writer->write(x, y, {0xFF, 0xFF, 0xFF});
		}
	}

	for (int x = 0; x < 200; ++x) {
		for (int y = 0; y < 100; ++y) {
			pixel_writer->write(x, y, {0x0, 0xFF, 0x0});
		}
	}

	write_string(*pixel_writer, 30, 30, u8"chino chan kawaii!", {0x0, 0x0, 0x0});
	write_string(*pixel_writer, 30, 60, u8"gochuumon wa usagi desu ka?", {0x0, 0x0, 0x0});

	while (true) {
		__asm("hlt");
	}
}
