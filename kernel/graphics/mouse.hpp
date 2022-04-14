#pragma once

#include "layer.hpp"
#include "window.hpp"

std::shared_ptr<Window> make_mouse_window(PixelFormat pixel_format);
BufferLayer* make_mouse_layer(LayerManager& manager);
