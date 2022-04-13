#include <cstring>

#include "console.hpp"
#include "font.hpp"
#include "graphics/layer.hpp"

Console::Console(const PixelColor& fg_color, const PixelColor& bg_color) :
	fg_color_(fg_color), bg_color_(bg_color), buffer_{}, cursor_row_(0), cursor_column_(0) {}

void Console::set_pixel_writer(PixelWriter* writer) {
	writer_ = writer;
	redraw();
}

void Console::put_string(const char* s) {
	for (; *s != u8'\0'; ++s) {
		if (*s == u8'\n') {
			new_line();
			continue;
		}

	retry:
		if (cursor_column_ < columns) {
			if (writer_ != nullptr) {
				draw_char_at(cursor_column_, cursor_row_, *s);
			}
			buffer_[cursor_row_][cursor_column_] = *s;
			++cursor_column_;
		} else {
			new_line();
			goto retry;
		}
	}

	if (layer_manager != nullptr) {
		layer_manager->draw();
	}
}

void Console::new_line() {
	cursor_column_ = 0;

	if (cursor_row_ < rows - 1) {
		++cursor_row_;
	} else {
		std::memmove(buffer_[0], buffer_[1], columns * (rows - 1));
		std::memset(buffer_[rows - 1], u8'\0', columns);
		redraw();
	}
}

void Console::redraw() {
	if (writer_ == nullptr) {
		return;
	}

	for (int y = 0; y < 16 * rows; ++y) {
		for (int x = 0; x < 8 * columns; ++x) {
			writer_->write(x, y, bg_color_);
		}
	}

	for (int y = 0; y < rows; ++y) {
		for (int x = 0; x < columns; ++x) {
			draw_char_at(x, y, buffer_[y][x]);
		}
	}
}

void Console::draw_char_at(int x, int y, char c) {
	if (c == u8'\0') {
		return;
	}

	write_ascii(*writer_, x * 8, y * 16, c, fg_color_);
}
