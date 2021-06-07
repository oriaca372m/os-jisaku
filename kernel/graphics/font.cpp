#include "font.hpp"

#include <cstdint>

extern const std::uint8_t _binary_hankaku_bin_start;
extern const std::uint8_t _binary_hankaku_bin_end;
extern const std::uint8_t _binary_hankaku_bin_size;

namespace {
	const std::uint8_t* get_font(char c) {
		auto index = 16 * static_cast<unsigned int>(c);
		if (index >= reinterpret_cast<uintptr_t>(&_binary_hankaku_bin_size)) {
			return nullptr;
		}

		return &_binary_hankaku_bin_start + index;
	}
}

void write_ascii(PixelWriter& writer, int x, int y, char c, const PixelColor& color) {
	const auto font = get_font(c);
	if (font == nullptr) {
		return;
	}

	for (int dy = 0; dy < 16; ++dy) {
		for (int dx = 0; dx < 8; ++dx) {
			if ((font[dy] << dx & 0b1000'0000) != 0) {
				writer.write(x + dx, y + dy, color);
			}
		}
	}
}

void write_string(PixelWriter& writer, int x, int y, const char* s, const PixelColor& color) {
	for (int i = 0; s[i] != u8'\0'; ++i) {
		write_ascii(writer, x + i * 8, y, s[i], color);
	}
}
