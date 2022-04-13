#pragma once

#include <cstdint>
#include <memory>

#include "frame_buffer_config.hpp"

struct PixelColor {
	std::uint8_t r;
	std::uint8_t g;
	std::uint8_t b;

	bool operator==(const PixelColor& other) const {
		return r == other.r && g == other.g && b == other.b;
	}

	bool operator!=(const PixelColor& other) const {
		return !(*this == other);
	}
};

template <typename T>
struct Vector2D {
	T x;
	T y;

	template <typename U>
	Vector2D<T>& operator+=(const Vector2D<U>& rhs) {
		x += rhs.x;
		y += rhs.y;
		return *this;
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
