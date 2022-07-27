#pragma once

#include "frame_buffer.hpp"
#include "layer.hpp"

#include <memory>
#include <utility>
#include <vector>

namespace graphics {
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

		void move(LayerId id, Vector2D<int> new_position);
		void move_relative(LayerId id, Vector2D<int> pos_diff);

		void up_down(LayerId id, int new_height);
		void hide(LayerId id);

		Layer* find_layer(LayerId id) const;
		Layer* find_layer_by_position(Vector2D<int> pos, LayerId exclude_id) const;

		// idを持つ属するLayerのコンテンツの範囲rectsが更新された時呼ばれる
		// rectsはこのLayerManagerの座標空間
		virtual void damage(LayerId id, const std::vector<Rect<int>>& rects) const;

	protected:
		FrameBuffer* buffer_ = nullptr;
		Layer* parent_ = nullptr;
		std::vector<Layer*> layer_stack_{};

		void draw_to(FrameBuffer& buffer) const;
		void draw_damage_to(FrameBuffer& buffer, LayerId id, const Rect<int>& rects) const;
		Rect<int> merge_rects(const std::vector<Rect<int>>& rects) const;

	private:
		const PixelFormat pixel_format_;

		std::vector<std::unique_ptr<Layer>> layers_{};
		LayerId latest_id_ = 0;

		decltype(layer_stack_)::iterator find_layer_stack_itr(LayerId id);
		decltype(layer_stack_)::iterator find_layer_stack_itr(LayerId id, decltype(layer_stack_)::iterator begin);
		decltype(layer_stack_)::const_iterator
		find_layer_stack_itr(LayerId id, decltype(layer_stack_)::const_iterator begin) const;
	};

	class DoubleBufferedLayerManager final : public LayerManager {
		using LayerManager::LayerManager;

		void set_buffer(FrameBuffer* buffer) override;

		void draw() const override;
		void damage(LayerId id, const std::vector<Rect<int>>& rects) const override;

	private:
		mutable std::optional<FrameBuffer> back_buffer_;
	};

	inline LayerManager* layer_manager = nullptr;
}
