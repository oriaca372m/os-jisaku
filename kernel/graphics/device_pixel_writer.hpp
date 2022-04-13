#pragma once

#include "graphics.hpp"

class DevicePixelWriter : public PixelWriter {
public:
	DevicePixelWriter(const FrameBufferConfig& config) : config_{config} {};

protected:
	std::uint8_t* pixel_at(int x, int y);

private:
	const FrameBufferConfig config_;
};

class RGBResv8BitPerColorPixelWriter final : public DevicePixelWriter {
	using DevicePixelWriter::DevicePixelWriter;

	void write(int x, int y, const PixelColor& c) override;
};

class BGRResv8BitPerColorPixelWriter final : public DevicePixelWriter {
	using DevicePixelWriter::DevicePixelWriter;

	void write(int x, int y, const PixelColor& c) override;
};

std::unique_ptr<DevicePixelWriter> make_suitable_device_pixel_writer(const FrameBufferConfig& config);
