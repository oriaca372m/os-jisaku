#include "layer.hpp"

#include <algorithm>
#include <memory>

Layer::Layer(LayerManager& manager, unsigned int id) : manager_(manager), id_{id} {}

unsigned int Layer::id() const {
	return id_;
}

Vector2D<int> Layer::pos() const {
	return pos_;
}

Vector2D<int> Layer::size() const {
	return rendered_->size();
}

void Layer::set_transparent_color(std::optional<PixelColor> c) {
	transparent_color_ = c;
}

Rect<int> Layer::manager_area() const {
	return Rect<int>(pos_, pos_ + size());
}

Layer& Layer::move(Vector2D<int> pos) {
	const auto before = manager_area();
	pos_ = pos;
	const auto after = manager_area();
	manager_.damage({before, after});
	return *this;
}

Layer& Layer::move_relative(Vector2D<int> pos_diff) {
	return move(pos_ + pos_diff);
}

void Layer::draw_to(FrameBuffer& dst) const {
	dst.copy_from(*rendered_, pos_);
}

void Layer::draw_to(FrameBuffer& dst, Rect<int> damage) const {
	const auto layer_rect = manager_area();
	if (!damage.is_crossing(layer_rect)) {
		return;
	}

	const auto cross = damage.cross(layer_rect);
	const auto src_pos = cross.top_left - pos_;

	dst.copy_from(*rendered_, cross.top_left, src_pos, {cross.width(), cross.height()});
}

void Layer::damage(const std::vector<Rect<int>>& rects) {
	std::vector<Rect<int>> res;
	std::transform(rects.cbegin(), rects.cend(), std::back_inserter(res), [this](auto x) { return x.offset(pos_); });
	manager_.damage(res);
};

Painter::Painter(FrameBuffer& buffer, Layer& layer) : buffer_(buffer), layer_(layer) {}
Painter::~Painter() {
	end();
}

void Painter::end() {
	layer_.damage(damages_);
	damages_.clear();
}

void Painter::draw_rectangle(const Rect<int>& rect, const PixelColor& c) {
	::draw_rectangle(raw_pixel_writer(), rect.top_left, rect.size(), c);
	raw_damage(rect);
}
void Painter::draw_filled_rectangle(const Rect<int>& rect, const PixelColor& c) {
	::draw_filled_rectangle(raw_pixel_writer(), rect.top_left, rect.size(), c);
	raw_damage(rect);
}

PixelWriter& Painter::pixel_writer() {
	raw_damage({{0, 0}, layer_.size()});
	return buffer_.writer();
}

PixelWriter& Painter::raw_pixel_writer() {
	return buffer_.writer();
}

void Painter::raw_damage(const Rect<int>& rect) {
	damages_.push_back(rect);
}

BufferLayer::BufferLayer(LayerManager& manager, unsigned int id, const FrameBufferConfig& config) : Layer(manager, id) {
	rendered_ = std::make_unique<FrameBuffer>(config);
}

Painter BufferLayer::start_paint() {
	return Painter(*rendered_, *this);
}
