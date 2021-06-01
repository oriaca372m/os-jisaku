#include <cstddef>
#include <cstdint>

#include "frame_buffer_config.hpp"

struct PixelColor {
	std::uint8_t r;
	std::uint8_t g;
	std::uint8_t b;
};

class PixelWriter {
public:
	PixelWriter(const FrameBufferConfig& config) : config_{config} {}
	virtual ~PixelWriter() = default;

	virtual void write(int x, int y, const PixelColor& c) = 0;

protected:
	std::uint8_t* pixel_at(int x, int y) {
		const int pixel_position = config_.pixels_per_scan_line * y + x;
		return config_.frame_buffer + 4 * pixel_position;
	}

private:
	const FrameBufferConfig config_;
};

class RGBResv8BitPerColorPixelWriter final : public PixelWriter {
	using PixelWriter::PixelWriter;

	void write(int x, int y, const PixelColor& c) override {
		auto p = pixel_at(x, y);
		p[0] = c.r;
		p[1] = c.g;
		p[2] = c.b;
	}
};

class BGRResv8BitPerColorPixelWriter final : public PixelWriter {
	using PixelWriter::PixelWriter;

	void write(int x, int y, const PixelColor& c) override {
		auto p = pixel_at(x, y);
		p[0] = c.b;
		p[1] = c.g;
		p[2] = c.r;
	}
};

void* operator new(std::size_t, void* buf) {
	return buf;
}

void operator delete(void*) noexcept {}

extern "C" void __cxa_pure_virtual() {
	while (true) {
		__asm("hlt");
	}
}

extern "C" void KernelMain(const FrameBufferConfig& frame_buffer_config) {
	char pixel_writer_buf[sizeof(RGBResv8BitPerColorPixelWriter)];
	PixelWriter* pixel_writer = reinterpret_cast<PixelWriter*>(pixel_writer_buf);

	if (frame_buffer_config.pixel_format == PixelFormat::RGBResv8BitPerColor) {
		new (pixel_writer_buf) RGBResv8BitPerColorPixelWriter(frame_buffer_config);
	} else if (frame_buffer_config.pixel_format == PixelFormat::BGRResv8BitPerColor) {
		new (pixel_writer_buf) BGRResv8BitPerColorPixelWriter(frame_buffer_config);
	}

	for (int x = 0; x < frame_buffer_config.horizontal_resolution; ++x) {
		for (int y = 0; y < frame_buffer_config.vertical_resolution; ++y) {
			pixel_writer->write(x, y, {0xFF, 0xFF, 0xFF});
		}
	}

	for (int x = 0; x < 200; ++x) {
		for (int y = 0; y < 100; ++y) {
			pixel_writer->write(x, y, {0x0, 0xFF, 0x0});
		}
	}

	while (true) {
		__asm("hlt");
	}
}
