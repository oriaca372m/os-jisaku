#pragma once

#include <memory>
#include <optional>

#include "frame_buffer.hpp"
#include "painter.hpp"

namespace graphics {
	using LayerId = unsigned int;

	class LayerManager;

	class Layer {
	public:
		Layer(LayerManager& manager, LayerId id);
		virtual ~Layer() = default;

		LayerId id() const;
		virtual Vector2D<int> pos() const;
		virtual Vector2D<int> size() const = 0;

		void set_transparent_color(std::optional<PixelColor> c);
		bool has_transparency() const;

		void move(Vector2D<int> pos);
		void move_relative(Vector2D<int> pos_diff);

		virtual void draw_to(FrameBuffer& dst) const = 0;
		virtual void draw_to(FrameBuffer& dst, Rect<int> damage) const = 0;

		// このLayerのコンテンツの範囲rectsが更新された時呼ばれる
		// rectsはこのLayerの座標空間
		void damage(const std::vector<Rect<int>>& rects);

		// 属しているLayerManagerの座標空間でこのLayerが表示される領域
		Rect<int> manager_area() const;

		void set_draggable(bool draggable);
		bool is_draggable() const;

	protected:
		LayerManager& manager_;
		const LayerId id_;
		Vector2D<int> pos_ = {0, 0};
		std::optional<PixelColor> transparent_color_;
		bool draggable_ = false;
	};
}
