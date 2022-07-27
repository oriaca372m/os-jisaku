#include "group_layer.hpp"

using graphics::GroupLayer;
using graphics::LayerManager;
using graphics::Vector2D;

GroupLayer::GroupLayer(LayerManager& manager, LayerId id, PixelFormat pixel_format, Vector2D<int> size) :
	Layer(manager, id), child_manager(pixel_format), buffer_(FrameBufferConfig(size.x, size.y, pixel_format)) {
	child_manager.set_parent(this);
	child_manager.set_buffer(&buffer_);
}

Vector2D<int> GroupLayer::size() const {
	return buffer_.size();
}

void GroupLayer::draw_to(FrameBuffer& dst) const {
	dst.copy_from(buffer_, pos_, transparent_color_);
}

void GroupLayer::draw_to(FrameBuffer& dst, Rect<int> damage) const {
	const auto layer_rect = manager_area();
	if (!damage.is_crossing(layer_rect)) {
		return;
	}

	const auto cross = damage.cross(layer_rect);
	dst.copy_from(buffer_, cross.top_left(), cross.top_left() - pos_, cross.size(), transparent_color_);
}

LayerManager& GroupLayer::layer_manager() {
	return child_manager;
}
