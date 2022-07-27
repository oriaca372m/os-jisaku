#pragma once

#include <cstdint>

struct PixelColor {
	std::uint8_t r;
	std::uint8_t g;
	std::uint8_t b;

	constexpr PixelColor(std::uint8_t r, std::uint8_t g, std::uint8_t b) : r(r), g(g), b(b){};
	constexpr PixelColor(std::uint32_t c) :
		r(static_cast<std::uint8_t>((c >> 16) & 0xff)),
		g(static_cast<std::uint8_t>((c >> 8) & 0xff)),
		b(static_cast<std::uint8_t>(c & 0xff)) {}

	constexpr bool operator==(const PixelColor& other) const {
		return r == other.r && g == other.g && b == other.b;
	}

	constexpr bool operator!=(const PixelColor& other) const {
		return !(*this == other);
	}
};
