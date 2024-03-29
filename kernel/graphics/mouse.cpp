#include "mouse.hpp"

using graphics::BufferLayer;
using graphics::PixelWriter;
using graphics::Vector2D;

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

	void draw_mouse_cursor(PixelWriter& writer, Vector2D<int> pos) {
		for (int dy = 0; dy < mouse_cursor_height; ++dy) {
			for (int dx = 0; dx < mouse_cursor_width; ++dx) {
				auto c = mouse_cursor_shape[dy][dx];
				if (c == '@') {
					writer.write(pos.x + dx, pos.y + dy, {0x00, 0x00, 0x00});
				} else if (c == '.') {
					writer.write(pos.x + dx, pos.y + dy, {0xff, 0xff, 0xff});
				} else {
					writer.write(pos.x + dx, pos.y + dy, {0xff, 0x00, 0x00});
				}
			}
		}
	}
}

BufferLayer* graphics::make_mouse_layer(LayerManager& manager) {
	auto layer = manager.new_layer<BufferLayer>(Vector2D<int>(mouse_cursor_width, mouse_cursor_height));
	layer->set_transparent_color(PixelColor{0xff, 0x00, 0x00});
	draw_mouse_cursor(layer->start_paint().pixel_writer(), {0, 0});
	return layer;
}
