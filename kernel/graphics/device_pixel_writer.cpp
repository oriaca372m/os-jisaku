#include "device_pixel_writer.hpp"

std::uint8_t* DevicePixelWriter::pixel_at(int x, int y) {
	const int pixel_position = config_.pixels_per_scan_line * y + x;
	return config_.frame_buffer + 4 * pixel_position;
}

void RGBResv8BitPerColorPixelWriter::write(int x, int y, const PixelColor& c) {
	auto p = pixel_at(x, y);
	p[0] = c.r;
	p[1] = c.g;
	p[2] = c.b;
}

void BGRResv8BitPerColorPixelWriter::write(int x, int y, const PixelColor& c) {
	auto p = pixel_at(x, y);
	p[0] = c.b;
	p[1] = c.g;
	p[2] = c.r;
}

const RGBResv8BitPerColorPixelWriterTraits RGBResv8BitPerColorPixelWriterTraits::instance{};
const BGRResv8BitPerColorPixelWriterTraits BGRResv8BitPerColorPixelWriterTraits::instance{};

const DevicePixelWriterTraits& get_suitable_device_pixel_writer_traits(PixelFormat pixel_format) {
	switch (pixel_format) {
	case PixelFormat::RGBResv8BitPerColor:
		return RGBResv8BitPerColorPixelWriterTraits::instance;
	case PixelFormat::BGRResv8BitPerColor:
		return BGRResv8BitPerColorPixelWriterTraits::instance;
	}
}
