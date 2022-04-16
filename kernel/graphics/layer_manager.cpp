#include "layer_manager.hpp"

#include "layer.hpp"

LayerManager::LayerManager(PixelFormat pixel_format) : pixel_format_(pixel_format){};

void LayerManager::set_buffer(FrameBuffer* buffer) {
	buffer_ = buffer;
}

void LayerManager::set_parent(Layer* parent) {
	parent_ = parent;
}

BufferLayer* LayerManager::new_buffer_layer(Vector2D<int> size) {
	++latest_id_;
	auto layer = std::make_unique<BufferLayer>(*this, latest_id_, FrameBufferConfig(size.x, size.y, pixel_format_));
	auto layer_raw_ptr = layer.get();
	layers_.push_back(std::move(layer));
	return layer_raw_ptr;
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
	if (buffer_ == nullptr) {
		return;
	}

	for (const auto& layer : layer_stack_) {
		layer->draw_to(*buffer_);
	}
}

void LayerManager::damage(unsigned int layer_id, const std::vector<Rect<int>>& rects) {
	if (buffer_ == nullptr || rects.empty()) {
		return;
	}

	auto merged_rect = rects[0];
	for (std::size_t i = 1; i < rects.size(); ++i) {
		merged_rect = merged_rect.merge(rects[i]);
	}

	auto i = layer_stack_.end();
	while (i != layer_stack_.begin()) {
		--i;

		if ((*i)->has_transparency()) {
			continue;
		}

		if ((*i)->manager_area().includes(merged_rect)) {
			break;
		}
	}

	{
		const auto damaged_layer =
			std::find_if(i, layer_stack_.end(), [layer_id](auto layer) { return layer->id() == layer_id; });
		if (damaged_layer == layer_stack_.end()) {
			return;
		}
	}

	for (; i != layer_stack_.end(); ++i) {
		(*i)->draw_to(*buffer_, merged_rect);
	}

	if (parent_ != nullptr) {
		parent_->damage(rects);
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
