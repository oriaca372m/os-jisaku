#pragma once

#include "graphics.hpp"

class Console final {
public:
	static const int rows = 25;
	static const int columns = 80;

	Console(const PixelColor& fg_color, const PixelColor& bg_color);
	void set_pixel_writer(PixelWriter* writer);

	void put_string(const char* s);
	void redraw();

private:
	void new_line();
	void draw_char_at(int x, int y, char c);

	PixelWriter* writer_ = nullptr;
	const PixelColor fg_color_;
	const PixelColor bg_color_;
	char buffer_[rows][columns];
	int cursor_row_;
	int cursor_column_;
};

inline Console* global_console;
