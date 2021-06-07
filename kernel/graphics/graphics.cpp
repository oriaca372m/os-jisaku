#include "graphics.hpp"

std::uint8_t* DevicePixelWriter::pixel_at(int x, int y) {
	const int pixel_position = config_.pixels_per_scan_line * y + x;
	return config_.frame_buffer + 4 * pixel_position;
}

void RGBResv8BitPerColorPixelWriter::write(int x, int y, const PixelColor& c) {
	auto p = pixel_at(x, y);
	p[0] = c.r;
	p[1] = c.g;
	p[2] = c.b;
}

void BGRResv8BitPerColorPixelWriter::write(int x, int y, const PixelColor& c) {
	auto p = pixel_at(x, y);
	p[0] = c.b;
	p[1] = c.g;
	p[2] = c.r;
}

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
