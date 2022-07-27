#pragma once

#include "pixel_color.hpp"

class PixelWriter {
public:
	virtual ~PixelWriter() = default;

	virtual void write(int x, int y, const PixelColor& c) = 0;
};
