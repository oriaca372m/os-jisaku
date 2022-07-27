#include "buffer_layer.hpp"

using graphics::BufferLayer;
using graphics::Painter;
using graphics::Vector2D;

BufferLayer::BufferLayer(LayerManager& manager, LayerId id, PixelFormat pixel_format, Vector2D<int> size) :
	Layer(manager, id), buffer_(FrameBufferConfig(size.x, size.y, pixel_format)) {}

Vector2D<int> BufferLayer::size() const {
	return buffer_.size();
}

void BufferLayer::draw_to(FrameBuffer& dst) const {
	dst.copy_from(buffer_, pos_, transparent_color_);
}

void BufferLayer::draw_to(FrameBuffer& dst, Rect<int> damage) const {
	const auto layer_rect = manager_area();
	if (!damage.is_crossing(layer_rect)) {
		return;
	}

	const auto cross = damage.cross(layer_rect);
	dst.copy_from(buffer_, cross.top_left(), cross.top_left() - pos_, cross.size(), transparent_color_);
}

Painter BufferLayer::start_paint() {
	return Painter(buffer_, *this);
}
