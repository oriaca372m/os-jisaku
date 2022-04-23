#include "painter.hpp"

#include "font.hpp"
#include "layer.hpp"

Painter::Painter(FrameBuffer& buffer, Layer& layer) : buffer_(buffer), layer_(layer) {}
Painter::~Painter() {
	end();
}

void Painter::end() {
	if (!damaged_) {
		return;
	}

	layer_.damage({{damage_top_left_, damage_bottom_right_}});
}

void Painter::copy_y(int dst_y, int src_y, int src_end_y) {
	const auto height = src_end_y - src_y;
	buffer_.copy_self_y(dst_y, src_y, height);
	raw_damage({0, dst_y, static_cast<int>(buffer_.size().x), dst_y + height});
}

void Painter::draw_rectangle(const Rect<int>& rect, const PixelColor& c) {
	::draw_rectangle(raw_pixel_writer(), rect.top_left(), rect.size(), c);
	raw_damage(rect);
}

void Painter::draw_filled_rectangle(const Rect<int>& rect, const PixelColor& c) {
	::draw_filled_rectangle(raw_pixel_writer(), rect.top_left(), rect.size(), c);
	raw_damage(rect);
}

void Painter::draw_ascii(const Vector2D<int>& pos, char ch, const PixelColor& c) {
	write_ascii(raw_pixel_writer(), pos.x, pos.y, ch, c);
	raw_damage(Rect<int>::with_size(pos, {8, 16}));
}

PixelWriter& Painter::pixel_writer() {
	raw_damage({{0, 0}, layer_.size()});
	return buffer_.writer();
}

PixelWriter& Painter::raw_pixel_writer() {
	return buffer_.writer();
}

FrameBuffer& Painter::raw_buffer() {
	return buffer_;
}

void Painter::raw_damage(const Rect<int>& rect) {
	if (!damaged_) {
		damage_top_left_ = rect.top_left();
		damage_bottom_right_ = rect.bottom_right();
		damaged_ = true;
		return;
	}

	damage_top_left_ = damage_top_left_.min(rect.top_left());
	damage_bottom_right_ = damage_bottom_right_.max(rect.bottom_right());
}
