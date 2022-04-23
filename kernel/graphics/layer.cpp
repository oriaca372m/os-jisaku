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

void Layer::set_transparent_color(std::optional<PixelColor> c) {
	transparent_color_ = c;
}

bool Layer::has_transparency() const {
	return transparent_color_.has_value();
}

Rect<int> Layer::manager_area() const {
	return Rect<int>(pos_, pos_ + size());
}

void Layer::move(Vector2D<int> pos) {
	const auto before = manager_area();
	pos_ = pos;
	const auto after = manager_area();
	manager_.damage(id_, {before, after});
}

void Layer::move_relative(Vector2D<int> pos_diff) {
	move(pos_ + pos_diff);
}

void Layer::damage(const std::vector<Rect<int>>& rects) {
	std::vector<Rect<int>> res;
	std::transform(rects.cbegin(), rects.cend(), std::back_inserter(res), [this](auto x) { return x.offset(pos_); });
	manager_.damage(id_, res);
};

BufferLayer::BufferLayer(LayerManager& manager, unsigned int id, PixelFormat pixel_format, Vector2D<int> size) :
	Layer(manager, id), buffer_(FrameBufferConfig(size.x, size.y, pixel_format)) {}

Vector2D<int> BufferLayer::size() const {
	return buffer_.size();
}

void BufferLayer::draw_to(FrameBuffer& dst) const {
	dst.copy_from(buffer_, pos_, transparent_color_);
}

void BufferLayer::draw_to(FrameBuffer& dst, Rect<int> damage) const {
	const auto layer_rect = manager_area();
	if (!damage.is_crossing(layer_rect)) {
		return;
	}

	const auto cross = damage.cross(layer_rect);
	dst.copy_from(buffer_, cross.top_left(), cross.top_left() - pos_, cross.size(), transparent_color_);
}

Painter BufferLayer::start_paint() {
	return Painter(buffer_, *this);
}

GroupLayer::GroupLayer(LayerManager& manager, unsigned int id, PixelFormat pixel_format, Vector2D<int> size) :
	Layer(manager, id), child_manager(pixel_format), buffer_(FrameBufferConfig(size.x, size.y, pixel_format)) {
	child_manager.set_parent(this);
	child_manager.set_buffer(&buffer_);
}

Vector2D<int> GroupLayer::size() const {
	return buffer_.size();
}

void GroupLayer::draw_to(FrameBuffer& dst) const {
	dst.copy_from(buffer_, pos_, transparent_color_);
}

void GroupLayer::draw_to(FrameBuffer& dst, Rect<int> damage) const {
	const auto layer_rect = manager_area();
	if (!damage.is_crossing(layer_rect)) {
		return;
	}

	const auto cross = damage.cross(layer_rect);
	dst.copy_from(buffer_, cross.top_left(), cross.top_left() - pos_, cross.size(), transparent_color_);
}

LayerManager& GroupLayer::layer_manager() {
	return child_manager;
}
