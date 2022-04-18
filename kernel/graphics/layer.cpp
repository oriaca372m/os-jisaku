#include "layer.hpp"

#include <algorithm>
#include <memory>

#include "font.hpp"

Layer::Layer(LayerManager& manager, unsigned int id) : manager_(manager), id_{id} {}

unsigned int Layer::id() const {
	return id_;
}

Vector2D<int> Layer::pos() const {
	return pos_;
}

void Layer::set_transparent_color(std::optional<PixelColor> c) {
	transparent_color_ = c;
}

bool Layer::has_transparency() const {
	return transparent_color_.has_value();
}

Rect<int> Layer::manager_area() const {
	return Rect<int>(pos_, pos_ + size());
}

Layer& Layer::move(Vector2D<int> pos) {
	const auto before = manager_area();
	pos_ = pos;
	const auto after = manager_area();
	manager_.damage(id_, {before, after});
	return *this;
}

Layer& Layer::move_relative(Vector2D<int> pos_diff) {
	return move(pos_ + pos_diff);
}

void Layer::damage(const std::vector<Rect<int>>& rects) {
	std::vector<Rect<int>> res;
	std::transform(rects.cbegin(), rects.cend(), std::back_inserter(res), [this](auto x) { return x.offset(pos_); });
	manager_.damage(id_, res);
};

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

BufferLayer::BufferLayer(LayerManager& manager, unsigned int id, const FrameBufferConfig& config) : Layer(manager, id) {
	buffer_ = std::make_unique<FrameBuffer>(config);
}

Vector2D<int> BufferLayer::size() const {
	return buffer_->size();
}

void BufferLayer::draw_to(FrameBuffer& dst) const {
	dst.copy_from(*buffer_, pos_, transparent_color_);
}

void BufferLayer::draw_to(FrameBuffer& dst, Rect<int> damage) const {
	const auto layer_rect = manager_area();
	if (!damage.is_crossing(layer_rect)) {
		return;
	}

	const auto cross = damage.cross(layer_rect);
	dst.copy_from(*buffer_, cross.top_left(), cross.top_left() - pos_, cross.size(), transparent_color_);
}

Painter BufferLayer::start_paint() {
	return Painter(*buffer_, *this);
}
