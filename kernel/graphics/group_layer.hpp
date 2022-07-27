#pragma once

#include "layer.hpp"
#include "layer_manager.hpp"

namespace graphics {
	class GroupLayer final : public Layer {
	public:
		GroupLayer(LayerManager& manager, LayerId id, PixelFormat pixel_format, Vector2D<int> size);

		Vector2D<int> size() const override;

		void draw_to(FrameBuffer& dst) const override;
		void draw_to(FrameBuffer& dst, Rect<int> damage) const override;

		LayerManager& layer_manager();

	private:
		LayerManager child_manager;
		FrameBuffer buffer_;
	};
}
