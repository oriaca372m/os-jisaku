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

Console* console;

template <typename... Args>
int printk(const char* format, Args... args) {
	char buf[1024];

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wformat-security"
	auto result = std::snprintf(buf, sizeof(buf), format, args...);
#pragma clang diagnostic pop

	if (result < 0) {
		return result;
	}

	console->put_string(buf);
	return result;
}

extern "C" void KernelMain(const FrameBufferConfig& frame_buffer_config) {
	char pixel_writer_buf[sizeof(RGBResv8BitPerColorPixelWriter)];
	PixelWriter* pixel_writer = reinterpret_cast<PixelWriter*>(pixel_writer_buf);

	if (frame_buffer_config.pixel_format == PixelFormat::RGBResv8BitPerColor) {
		new (pixel_writer_buf) RGBResv8BitPerColorPixelWriter(frame_buffer_config);
	} else if (frame_buffer_config.pixel_format == PixelFormat::BGRResv8BitPerColor) {
		new (pixel_writer_buf) BGRResv8BitPerColorPixelWriter(frame_buffer_config);
	}

	Console console_instance(*pixel_writer, {0xc8, 0xc8, 0xc6}, {0x1d, 0x1f, 0x21});
	console = &console_instance;

	printk(u8"chino chan kawaii!\n");
	printk(u8"gochuumon wa usagi desu ka?\n");

	for (int i = 0; i < 100; i++) {
		printk("i = %d", i);
		printk(u8"chino chan kawaii!");

		for (volatile int j = 0; j < 100000000; j++) {
		}
	}

	while (true) {
		__asm("hlt");
	}
}
