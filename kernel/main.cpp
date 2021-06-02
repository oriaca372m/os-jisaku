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

namespace {
	const int mouse_cursor_width = 15;
	const int mouse_cursor_height = 24;
	const char mouse_cursor_shape[mouse_cursor_height][mouse_cursor_width + 1] = {
		"@              ", // keep shape
		"@@             ", // keep shape
		"@.@            ", // keep shape
		"@..@           ", // keep shape
		"@...@          ", // keep shape
		"@....@         ", // keep shape
		"@.....@        ", // keep shape
		"@......@       ", // keep shape
		"@.......@      ", // keep shape
		"@........@     ", // keep shape
		"@.........@    ", // keep shape
		"@..........@   ", // keep shape
		"@...........@  ", // keep shape
		"@............@ ", // keep shape
		"@......@@@@@@@@", // keep shape
		"@......@       ", // keep shape
		"@....@@.@      ", // keep shape
		"@...@ @.@      ", // keep shape
		"@..@   @.@     ", // keep shape
		"@.@    @.@     ", // keep shape
		"@@      @.@    ", // keep shape
		"@       @.@    ", // keep shape
		"         @.@   ", // keep shape
		"         @@@   ", // keep shape
	};

	void draw_mouse(PixelWriter& writer, int x, int y) {
		for (int dy = 0; dy < mouse_cursor_height; ++dy) {
			for (int dx = 0; dx < mouse_cursor_width; ++dx) {
				auto c = mouse_cursor_shape[dy][dx];
				if (c == '@') {
					writer.write(x + dx, y + dy, {0x00, 0x00, 0x00});
				} else if (c == '.') {
					writer.write(x + dx, y + dy, {0xFF, 0xFF, 0xFF});
				}
			}
		}
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

	const PixelColor desktop_fg_color{0xc8, 0xc8, 0xc6};
	const PixelColor desktop_bg_color{0x1d, 0x1f, 0x21};
	Console console_instance(*pixel_writer, desktop_fg_color, desktop_bg_color);
	console = &console_instance;

	const int frame_width = frame_buffer_config.horizontal_resolution;
	const int frame_height = frame_buffer_config.vertical_resolution;

	draw_filled_rectangle(*pixel_writer, {0, 0}, {frame_width, frame_height - 50}, desktop_bg_color);
	draw_filled_rectangle(*pixel_writer, {0, frame_height - 50}, {frame_width, 50}, {1, 8, 17});
	draw_filled_rectangle(*pixel_writer, {0, frame_height - 50}, {frame_width / 5, 50}, {80, 80, 80});
	draw_rectangle(*pixel_writer, {10, frame_height - 40}, {30, 30}, {160, 160, 160});

	printk(u8"chino chan kawaii!\n");
	printk(u8"gochuumon wa usagi desu ka?\n");

	draw_mouse(*pixel_writer, 200, 100);

	while (true) {
		__asm("hlt");
	}
}
