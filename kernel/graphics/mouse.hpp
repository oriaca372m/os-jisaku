#pragma once

#include "layer.hpp"

namespace graphics {
	BufferLayer* make_mouse_layer(LayerManager& manager);
	inline unsigned int mouse_layer_id;
}
