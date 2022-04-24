#include "layer_manager.hpp"

#include "layer.hpp"

LayerManager::LayerManager(PixelFormat pixel_format) : pixel_format_(pixel_format){};

void LayerManager::set_buffer(FrameBuffer* buffer) {
	buffer_ = buffer;
}

void LayerManager::set_parent(Layer* parent) {
	parent_ = parent;
}

Layer* LayerManager::find_layer(unsigned int id) {
	const auto it = std::find_if(layers_.cbegin(), layers_.cend(), [id](const auto& elm) { return elm->id() == id; });
	if (it == layers_.end()) {
		return nullptr;
	}
	return it->get();
}

Layer* LayerManager::find_layer_by_position(Vector2D<int> pos, unsigned int exclude_id) const {
	auto it = std::find_if(layer_stack_.crbegin(), layer_stack_.crend(), [pos, exclude_id](Layer* layer) {
		if (layer->id() == exclude_id) {
			return false;
		}

		return layer->manager_area().includes(pos);
	});

	if (it == layer_stack_.crend()) {
		return nullptr;
	}
	return *it;
}

decltype(LayerManager::layer_stack_)::iterator LayerManager::find_layer_stack_itr(unsigned int id) {
	return find_layer_stack_itr(id, layer_stack_.begin());
}

decltype(LayerManager::layer_stack_)::iterator
LayerManager::find_layer_stack_itr(unsigned int id, decltype(layer_stack_)::iterator begin) {
	return std::find_if(begin, layer_stack_.end(), [id](const auto& elm) { return elm->id() == id; });
}

decltype(LayerManager::layer_stack_)::const_iterator
LayerManager::find_layer_stack_itr(unsigned int id, decltype(layer_stack_)::const_iterator begin) const {
	return std::find_if(begin, layer_stack_.cend(), [id](const auto& elm) { return elm->id() == id; });
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

	draw_to(*buffer_);
}

void LayerManager::draw_to(FrameBuffer& buffer) const {
	for (const auto& layer : layer_stack_) {
		layer->draw_to(buffer);
	}
}

void LayerManager::damage(unsigned int layer_id, const std::vector<Rect<int>>& rects) const {
	if (buffer_ == nullptr || rects.empty()) {
		return;
	}

	draw_damage_to(*buffer_, layer_id, merge_rects(rects));

	if (parent_ != nullptr) {
		parent_->damage(rects);
	}
}

void LayerManager::draw_damage_to(FrameBuffer& buffer, unsigned int layer_id, const Rect<int>& rect) const {
	// 不透明でdamage範囲を完全に含む最前面のレイヤーiを探す
	// それより背面にあるレイヤーはレイヤーiに隠されるので描画する必要が無い
	auto i = layer_stack_.end();
	while (i != layer_stack_.begin()) {
		--i;

		if ((*i)->has_transparency()) {
			continue;
		}

		if ((*i)->manager_area().includes(rect)) {
			break;
		}
	}

	// damageされたレイヤーがレイヤーi以上に前面になければ描画をスキップ
	if (find_layer_stack_itr(layer_id, i) == layer_stack_.end()) {
		return;
	}

	for (; i != layer_stack_.end(); ++i) {
		(*i)->draw_to(buffer, rect);
	}
}

Rect<int> LayerManager::merge_rects(const std::vector<Rect<int>>& rects) const {
	auto merged_rect = rects[0];
	for (std::size_t i = 1; i < rects.size(); ++i) {
		merged_rect = merged_rect.merge(rects[i]);
	}
	return merged_rect;
};

void LayerManager::hide(unsigned int id) {
	const auto pos = find_layer_stack_itr(id);
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
	const auto old_pos = find_layer_stack_itr(id);
	auto new_pos = layer_stack_.cbegin() + new_height;

	const auto insert = [this](const auto& pos, const auto& layer) {
		layer_stack_.insert(pos, layer);
		damage(layer->id(), {layer->manager_area()});
	};

	if (old_pos == layer_stack_.end()) {
		insert(new_pos, layer);
		return;
	}

	if (new_pos == layer_stack_.end()) {
		--new_pos;
	}

	layer_stack_.erase(old_pos);
	insert(new_pos, layer);
}

void DoubleBufferedLayerManager::set_buffer(FrameBuffer* buffer) {
	if (buffer == nullptr) {
		back_buffer_.reset();
	} else {
		back_buffer_ = buffer->clone();
	}

	LayerManager::set_buffer(buffer);
}

void DoubleBufferedLayerManager::draw() const {
	if (buffer_ == nullptr) {
		return;
	}

	draw_to(*back_buffer_);
	buffer_->forward(*back_buffer_);
}

void DoubleBufferedLayerManager::damage(unsigned int layer_id, const std::vector<Rect<int>>& rects) const {
	if (buffer_ == nullptr || rects.empty()) {
		return;
	}

	auto merged_rect = merge_rects(rects);
	draw_damage_to(*back_buffer_, layer_id, merged_rect);
	buffer_->copy_from(*back_buffer_, merged_rect.top_left(), merged_rect.top_left(), merged_rect.size(), std::nullopt);

	if (parent_ != nullptr) {
		parent_->damage(rects);
	}
}
