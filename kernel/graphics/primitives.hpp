#pragma once

#include <algorithm>

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

	template <typename U>
	Vector2D<T> operator-(const Vector2D<U>& rhs) const {
		return {x - rhs.x, y - rhs.y};
	}

	template <typename U>
	explicit operator Vector2D<U>() const {
		return {static_cast<U>(x), static_cast<U>(y)};
	}

	template <typename R = T, typename Operation, typename... U>
	Vector2D<R> map(Operation op, const Vector2D<U>&... other) {
		return {op(x, other.x...), op(y, other.y...)};
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

	constexpr Rect<T> cross(Rect<T> other) const {
		return {
			{std::max(left(), other.left()), std::max(top(), other.top())},
			{std::min(right(), other.right()), std::min(bottom(), other.bottom())}};
	}

	constexpr bool is_crossing(Rect<T> other) const {
		return left() < other.right() && top() < other.bottom() && other.left() < right() && other.top() < bottom();
	}
};