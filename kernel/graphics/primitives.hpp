#pragma once

#include <algorithm>

template <typename T>
struct Vector2D final {
	constexpr Vector2D(T x = 0, T y = 0) : x(x), y(y) {}

	T x;
	T y;

	constexpr Vector2D<T> operator+(const Vector2D<T>& rhs) const {
		return {x + rhs.x, y + rhs.y};
	}

	constexpr Vector2D<T> operator-(const Vector2D<T>& rhs) const {
		return {x - rhs.x, y - rhs.y};
	}

	constexpr Vector2D<T> min(const Vector2D<T>& other) const {
		return {std::min(x, other.x), std::min(y, other.y)};
	}

	constexpr Vector2D<T> max(const Vector2D<T>& other) const {
		return {std::max(x, other.x), std::max(y, other.y)};
	}

	template <typename U>
	constexpr explicit operator Vector2D<U>() const {
		return {static_cast<U>(x), static_cast<U>(y)};
	}

	template <typename R = T, typename Operation, typename... U>
	constexpr Vector2D<R> map(Operation op, const Vector2D<U>&... other) {
		return {op(x, other.x...), op(y, other.y...)};
	}
};

template <typename T>
struct Rect final {
	constexpr Rect(T left, T top, T right, T bottom) : left(left), top(top), right(right), bottom(bottom) {}
	constexpr Rect(const Vector2D<T>& top_left, const Vector2D<T>& bottom_right) :
		left(top_left.x), top(top_left.y), right(bottom_right.x), bottom(bottom_right.y) {}

	constexpr static Rect<T> with_size(const Vector2D<T>& top_left, const Vector2D<T>& size) {
		return {top_left, top_left + size};
	}

	T left;
	T top;
	T right;
	T bottom;

	constexpr Vector2D<T> top_left() const {
		return {left, top};
	};

	constexpr Vector2D<T> bottom_right() const {
		return {right, bottom};
	}

	constexpr T width() const {
		return right - left;
	}

	constexpr T height() const {
		return bottom - top;
	}

	constexpr Vector2D<T> size() const {
		return {width(), height()};
	}

	constexpr Rect<T> offset(const Vector2D<T>& diff) const {
		return {top_left() + diff, bottom_right() + diff};
	}

	constexpr Rect<T> merge(const Rect<T>& other) const {
		return {
			std::min(left, other.left),
			std::min(top, other.top),
			std::max(right, other.right),
			std::max(bottom, other.bottom)};
	}

	constexpr Rect<T> cross(const Rect<T>& other) const {
		return {
			std::max(left, other.left),
			std::max(top, other.top),
			std::min(right, other.right),
			std::min(bottom, other.bottom)};
	}

	constexpr bool is_crossing(const Rect<T>& other) const {
		return left < other.right && top < other.bottom && other.left < right && other.top < bottom;
	}

	// thisがotherを完全に囲んでいたらtrue
	constexpr bool includes(const Rect<T>& other) const {
		return left <= other.left && top <= other.top && other.right <= right && other.bottom <= bottom;
	}

	constexpr bool includes(Vector2D<T> p) const {
		return left <= p.x && top <= p.y && p.x < right && p.y < bottom;
	}
};
