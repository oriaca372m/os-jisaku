#pragma once

#include "layer.hpp"
#include "layer_manager.hpp"

namespace graphics {
	class BufferLayer final : public Layer {
	public:
		BufferLayer(LayerManager& manager, LayerId id, PixelFormat pixel_format, Vector2D<int> size);

		Vector2D<int> size() const override;

		void draw_to(FrameBuffer& dst) const override;
		void draw_to(FrameBuffer& dst, Rect<int> damage) const override;

		Painter start_paint();

	private:
		FrameBuffer buffer_;
	};
}
