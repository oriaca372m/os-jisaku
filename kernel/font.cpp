#include "font.hpp"

#include <cstdint>

namespace {
	const std::uint8_t fontA[16] = {
		0b00000000,
		0b00011000,
		0b00011000,
		0b00011000,
		0b00011000,
		0b00100100,
		0b00100100,
		0b00100100,
		0b00100100,
		0b01111110,
		0b01000010,
		0b01000010,
		0b01000010,
		0b11100111,
		0b00000000,
		0b00000000,
	};

}

void write_ascii(PixelWriter& writer, int x, int y, char c, const PixelColor& color) {
	if (c != 'A') {
		return;
	}

	for (int dy = 0; dy < 16; ++dy) {
		for (int dx = 0; dx < 8; ++dx) {
			if ((fontA[dy] << dx & 0b1000'0000) != 0) {
				writer.write(x + dx, y + dy, color);
			}
		}
	}
}
