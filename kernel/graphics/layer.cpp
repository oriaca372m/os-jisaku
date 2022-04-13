#include "layer.hpp"

#include <algorithm>
#include <memory>

Layer::Layer(unsigned int id) : id_{id} {}

unsigned int Layer::id() const {
	return id_;
}

Layer& Layer::set_window(const std::shared_ptr<Window>& window) {
	window_ = window;
	return *this;
}

std::shared_ptr<Window> Layer::get_window() const {
	return window_;
}

Layer& Layer::move(Vector2D<int> pos) {
	pos_ = pos;
	return *this;
}

Layer& Layer::move_relative(Vector2D<int> pos_diff) {
	pos_ += pos_diff;
	return *this;
}

const Vector2D<int>& Layer::pos() const {
	return pos_;
}

void Layer::draw_to(FrameBuffer& dst) const {
	window_->draw_to(dst, pos_);
}

void LayerManager::set_screen(FrameBuffer* screen) {
	screen_ = screen;
}

Layer& LayerManager::new_layer() {
	++latest_id_;
	return *layers_.emplace_back(std::make_unique<Layer>(latest_id_));
}

Layer* LayerManager::find_layer(unsigned int id) {
	const auto it = std::find_if(layers_.cbegin(), layers_.cend(), [id](const auto& elm) { return elm->id() == id; });
	if (it == layers_.end()) {
		return nullptr;
	}
	return it->get();
}

void LayerManager::move(unsigned int id, Vector2D<int> new_position) {
	find_layer(id)->move(new_position);
}

void LayerManager::move_relative(unsigned int id, Vector2D<int> pos_diff) {
	find_layer(id)->move_relative(pos_diff);
}

void LayerManager::draw() const {
	for (const auto& layer : layer_stack_) {
		layer->draw_to(*screen_);
	}
}

void LayerManager::hide(unsigned int id) {
	const auto layer = find_layer(id);
	const auto pos = std::find(layer_stack_.cbegin(), layer_stack_.cend(), layer);
	if (pos != layer_stack_.end()) {
		layer_stack_.erase(pos);
	}
}

void LayerManager::up_down(unsigned int id, int new_height) {
	if (new_height < 0) {
		hide(id);
		return;
	}

	if (new_height > layer_stack_.size()) {
		new_height = layer_stack_.size();
	}

	const auto layer = find_layer(id);
	const auto old_pos = std::find(layer_stack_.cbegin(), layer_stack_.cend(), layer);
	auto new_pos = layer_stack_.cbegin() + new_height;

	if (old_pos == layer_stack_.end()) {
		layer_stack_.insert(new_pos, layer);
		return;
	}

	if (new_pos == layer_stack_.end()) {
		--new_pos;
	}

	layer_stack_.erase(old_pos);
	layer_stack_.insert(new_pos, layer);
}
