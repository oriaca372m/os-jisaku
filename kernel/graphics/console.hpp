#pragma once

#include "layer.hpp"
#include "pixel_writer.hpp"

class IConsole {
public:
	virtual void put_string(const char* s) = 0;
};

class Console final : public IConsole {
public:
	static const int rows = 25;
	static const int columns = 80;

	Console(const PixelColor& fg_color, const PixelColor& bg_color);
	void set_pixel_writer(PixelWriter* writer);

	void put_string(const char* s) override;
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

class FastConsole final : public IConsole {
public:
	static const int rows = 25;
	static const int columns = 80;

	FastConsole(const PixelColor& fg_color, const PixelColor& bg_color, BufferLayer& layer);

	void put_string(const char* s) override;

private:
	void new_line(Painter& painter);

	BufferLayer& layer_;
	const PixelColor fg_color_;
	const PixelColor bg_color_;
	int cursor_row_;
	int cursor_column_;
};

inline IConsole* global_console;
