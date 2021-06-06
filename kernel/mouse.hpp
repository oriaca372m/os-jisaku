#pragma once

#include "graphics.hpp"

class MouseCursor final {
public:
	MouseCursor(PixelWriter* writer, PixelColor erase_color, Vector2D<int> initial_poosition);
	void move_relative(Vector2D<int> displacement);

private:
	PixelWriter* pixel_writer_;
	PixelColor erase_color_;
	Vector2D<int> position_;
};