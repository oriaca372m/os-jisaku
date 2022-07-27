#pragma once

#include <cstdint>

#include "frame_buffer_config.hpp"
#include "primitives.hpp"

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

class PixelWriter {
public:
	virtual ~PixelWriter() = default;

	virtual void write(int x, int y, const PixelColor& c) = 0;
};

void draw_filled_rectangle(
	PixelWriter& writer,
	const Vector2D<int>& pos,
	const Vector2D<int>& size,
	const PixelColor& c);
void draw_rectangle(PixelWriter& writer, const Vector2D<int>& pos, const Vector2D<int>& size, const PixelColor& c);

namespace logger {
	class ConsoleLogger;
}
void initialize_graphics(const FrameBufferConfig& frame_buffer_config, logger::ConsoleLogger& console_logger);

inline Vector2D<int> screen_size;

inline constexpr PixelColor desktop_fg_color{0xc8, 0xc8, 0xc6};
inline constexpr PixelColor desktop_bg_color{0x1d, 0x1f, 0x21};
