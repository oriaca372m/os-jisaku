#pragma once

#include <algorithm>
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

	template <typename U>
	Vector2D<T> operator+(const Vector2D<U>& rhs) const {
		return {x + rhs.x, y + rhs.y};
	}
};

template <typename T>
struct Rect {
	constexpr Rect(Vector2D<T> top_left, Vector2D<T> bottom_right) : top_left(top_left), bottom_right(bottom_right) {}

	const Vector2D<T> top_left;
	const Vector2D<T> bottom_right;

	constexpr T top() const {
		return top_left.y;
	}

	constexpr T left() const {
		return top_left.x;
	}

	constexpr T bottom() const {
		return bottom_right.y;
	}

	constexpr T right() const {
		return bottom_right.x;
	}

	constexpr T width() const {
		return bottom_right.x - top_left.x;
	}

	constexpr T height() const {
		return bottom_right.y - top_left.y;
	}

	constexpr Vector2D<T> size() const {
		return {width(), height()};
	}

	constexpr Rect<T> offset(Vector2D<T> diff) const {
		return {top_left + diff, bottom_right + diff};
	}

	constexpr Rect<T> merge(Rect<T> other) const {
		return {
			{std::min(left(), other.left()), std::min(top(), other.top())},
			{std::max(right(), other.right()), std::max(bottom(), other.bottom())}};
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
