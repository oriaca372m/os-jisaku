#pragma once

#include "frame_buffer.hpp"

#include <memory>
#include <utility>
#include <vector>

class Layer;

class LayerManager {
public:
	LayerManager(PixelFormat pixel_format);

	virtual void set_buffer(FrameBuffer* buffer);
	void set_parent(Layer* parent);

	template <typename T, typename... Args>
	T* new_layer(Args&&... args) {
		++latest_id_;
		auto layer = std::make_unique<T>(*this, latest_id_, pixel_format_, std::forward<Args>(args)...);
		auto layer_raw_ptr = layer.get();
		layers_.push_back(std::move(layer));
		return layer_raw_ptr;
	}

	virtual void draw() const;

	void move(unsigned int id, Vector2D<int> new_position);
	void move_relative(unsigned int id, Vector2D<int> pos_diff);

	void up_down(unsigned int id, int new_height);
	void hide(unsigned int id);

	Layer* find_layer(unsigned int id) const;
	Layer* find_layer_by_position(Vector2D<int> pos, unsigned int exclude_id) const;

	// layer_idを持つ属するLayerのコンテンツの範囲rectsが更新された時呼ばれる
	// rectsはこのLayerManagerの座標空間
	virtual void damage(unsigned int layer_id, const std::vector<Rect<int>>& rects) const;

protected:
	FrameBuffer* buffer_ = nullptr;
	Layer* parent_ = nullptr;
	std::vector<Layer*> layer_stack_{};

	void draw_to(FrameBuffer& buffer) const;
	void draw_damage_to(FrameBuffer& buffer, unsigned int layer_id, const Rect<int>& rects) const;
	Rect<int> merge_rects(const std::vector<Rect<int>>& rects) const;

private:
	const PixelFormat pixel_format_;

	std::vector<std::unique_ptr<Layer>> layers_{};
	unsigned int latest_id_ = 0;

	decltype(layer_stack_)::iterator find_layer_stack_itr(unsigned int id);
	decltype(layer_stack_)::iterator find_layer_stack_itr(unsigned int id, decltype(layer_stack_)::iterator begin);
	decltype(layer_stack_)::const_iterator
	find_layer_stack_itr(unsigned int id, decltype(layer_stack_)::const_iterator begin) const;
};

class DoubleBufferedLayerManager final : public LayerManager {
	using LayerManager::LayerManager;

	void set_buffer(FrameBuffer* buffer) override;

	void draw() const override;
	void damage(unsigned int layer_id, const std::vector<Rect<int>>& rects) const override;

private:
	mutable std::optional<FrameBuffer> back_buffer_;
};

inline LayerManager* layer_manager = nullptr;
inline unsigned int main_window_layer_id;
inline unsigned int group_layer_id;
inline unsigned int test_layer_id;
