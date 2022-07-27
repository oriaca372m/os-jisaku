#pragma once

#include <cstdint>

#include "frame_buffer_config.hpp"
#include "logger.hpp"
#include "primitives.hpp"

void draw_filled_rectangle(
	PixelWriter& writer,
	const Vector2D<int>& pos,
	const Vector2D<int>& size,
	const PixelColor& c);
void draw_rectangle(PixelWriter& writer, const Vector2D<int>& pos, const Vector2D<int>& size, const PixelColor& c);

void initialize_graphics(const FrameBufferConfig& frame_buffer_config, logger::ConsoleLogger& console_logger);

inline Vector2D<int> screen_size;

inline constexpr PixelColor desktop_fg_color{0xc8, 0xc8, 0xc6};
inline constexpr PixelColor desktop_bg_color{0x1d, 0x1f, 0x21};
