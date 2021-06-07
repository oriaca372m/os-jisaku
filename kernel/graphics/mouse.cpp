#include "mouse.hpp"
#include <memory>

namespace {
	const int mouse_cursor_width = 15;
	const int mouse_cursor_height = 24;
	const char mouse_cursor_shape[mouse_cursor_height][mouse_cursor_width + 1] = {
		u8"@              ", // keep shape
		u8"@@             ", // keep shape
		u8"@.@            ", // keep shape
		u8"@..@           ", // keep shape
		u8"@...@          ", // keep shape
		u8"@....@         ", // keep shape
		u8"@.....@        ", // keep shape
		u8"@......@       ", // keep shape
		u8"@.......@      ", // keep shape
		u8"@........@     ", // keep shape
		u8"@.........@    ", // keep shape
		u8"@..........@   ", // keep shape
		u8"@...........@  ", // keep shape
		u8"@............@ ", // keep shape
		u8"@......@@@@@@@@", // keep shape
		u8"@......@       ", // keep shape
		u8"@....@@.@      ", // keep shape
		u8"@...@ @.@      ", // keep shape
		u8"@..@   @.@     ", // keep shape
		u8"@.@    @.@     ", // keep shape
		u8"@@      @.@    ", // keep shape
		u8"@       @.@    ", // keep shape
		u8"         @.@   ", // keep shape
		u8"         @@@   ", // keep shape
	};

	void draw_mouse_cursor(PixelWriter* writer, Vector2D<int> pos) {
		for (int dy = 0; dy < mouse_cursor_height; ++dy) {
			for (int dx = 0; dx < mouse_cursor_width; ++dx) {
				auto c = mouse_cursor_shape[dy][dx];
				if (c == '@') {
					writer->write(pos.x + dx, pos.y + dy, {0x00, 0x00, 0x00});
				} else if (c == '.') {
					writer->write(pos.x + dx, pos.y + dy, {0xFF, 0xFF, 0xFF});
				} else {
					writer->write(pos.x + dx, pos.y + dy, {0xFF, 0x00, 0x00});
				}
			}
		}
	}
}

std::shared_ptr<Window> make_mouse_window() {
	const auto window = std::make_shared<Window>(mouse_cursor_width, mouse_cursor_height);
	window->set_transparent_color(PixelColor{0xFF, 0x00, 0x00});
	draw_mouse_cursor(window->writer(), {0, 0});
	return window;
}
