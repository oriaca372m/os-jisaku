#include "device_pixel_writer.hpp"

void RGBResv8BitPerColorPixelWriter::write(int x, int y, const PixelColor& c) {
	auto p = get_pixel_buf_at(x, y);
	p[0] = c.r;
	p[1] = c.g;
	p[2] = c.b;
}

PixelColor RGBResv8BitPerColorPixelWriter::get_pixel_at(int x, int y) const {
	auto p = get_pixel_buf_at(x, y);
	return {p[0], p[1], p[2]};
}

void BGRResv8BitPerColorPixelWriter::write(int x, int y, const PixelColor& c) {
	auto p = get_pixel_buf_at(x, y);
	p[0] = c.b;
	p[1] = c.g;
	p[2] = c.r;
}

PixelColor BGRResv8BitPerColorPixelWriter::get_pixel_at(int x, int y) const {
	auto p = get_pixel_buf_at(x, y);
	return {p[2], p[1], p[0]};
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
