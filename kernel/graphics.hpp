#pragma once

#include <cstdint>

#include "frame_buffer_config.hpp"

struct PixelColor {
	std::uint8_t r;
	std::uint8_t g;
	std::uint8_t b;
};

class PixelWriter {
public:
	PixelWriter(const FrameBufferConfig& config) : config_{config} {};
	virtual ~PixelWriter() = default;

	virtual void write(int x, int y, const PixelColor& c) = 0;

protected:
	std::uint8_t* pixel_at(int x, int y);

private:
	const FrameBufferConfig config_;
};

class RGBResv8BitPerColorPixelWriter final : public PixelWriter {
	using PixelWriter::PixelWriter;

	void write(int x, int y, const PixelColor& c) override;
};

class BGRResv8BitPerColorPixelWriter final : public PixelWriter {
	using PixelWriter::PixelWriter;

	void write(int x, int y, const PixelColor& c) override;
};