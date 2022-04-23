#pragma once

#include "frame_buffer.hpp"

#include <memory>
#include <utility>
#include <vector>

class Layer;

class LayerManager final {
public:
	LayerManager(PixelFormat pixel_format);

	void set_buffer(FrameBuffer* buffer);
	void set_parent(Layer* parent);

	template <typename T, typename... Args>
	T* new_layer(Args&&... args) {
		++latest_id_;
		auto layer = std::make_unique<T>(*this, latest_id_, pixel_format_, std::forward<Args>(args)...);
		auto layer_raw_ptr = layer.get();
		layers_.push_back(std::move(layer));
		return layer_raw_ptr;
	}

	void draw() const;

	void move(unsigned int id, Vector2D<int> new_position);
	void move_relative(unsigned int id, Vector2D<int> pos_diff);

	void up_down(unsigned int id, int new_height);
	void hide(unsigned int id);

	// layer_idを持つ属するLayerのコンテンツの範囲rectsが更新された時呼ばれる
	// rectsはこのLayerManagerの座標空間
	void damage(unsigned int layer_id, const std::vector<Rect<int>>& rects);

private:
	const PixelFormat pixel_format_;
	FrameBuffer* buffer_ = nullptr;
	Layer* parent_ = nullptr;

	std::vector<std::unique_ptr<Layer>> layers_{};
	std::vector<Layer*> layer_stack_{};
	unsigned int latest_id_ = 0;

	Layer* find_layer(unsigned int id);
	decltype(layer_stack_)::iterator find_layer_stack_itr(unsigned int id);
	decltype(layer_stack_)::iterator find_layer_stack_itr(unsigned int id, decltype(layer_stack_)::iterator begin);
};

inline LayerManager* layer_manager = nullptr;
