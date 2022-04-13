#include "graphics.hpp"

void draw_filled_rectangle(
	PixelWriter& writer,
	const Vector2D<int>& pos,
	const Vector2D<int>& size,
	const PixelColor& c) {
	for (int dy = 0; dy < size.y; ++dy) {
		for (int dx = 0; dx < size.x; ++dx) {
			writer.write(pos.x + dx, pos.y + dy, c);
		}
	}
}

void draw_rectangle(PixelWriter& writer, const Vector2D<int>& pos, const Vector2D<int>& size, const PixelColor& c) {
	for (int dx = 0; dx < size.x; ++dx) {
		writer.write(pos.x + dx, pos.y, c);
		writer.write(pos.x + dx, pos.y + size.y - 1, c);
	}

	for (int dy = 0; dy < size.y; ++dy) {
		writer.write(pos.x, pos.y + dy, c);
		writer.write(pos.x + size.x - 1, pos.y + dy, c);
	}
}
