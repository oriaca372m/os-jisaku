#include "layer.hpp"
#include "layer_manager.hpp"

#include <algorithm>
#include <memory>

using graphics::Layer;
using graphics::LayerId;
using graphics::Rect;
using graphics::Vector2D;

Layer::Layer(LayerManager& manager, LayerId id) : manager_(manager), id_{id} {}

LayerId Layer::id() const {
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

void Layer::set_draggable(bool draggable) {
	draggable_ = draggable;
}
bool Layer::is_draggable() const {
	return draggable_;
}
