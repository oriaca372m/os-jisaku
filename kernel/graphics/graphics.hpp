#pragma once

#include <cstdint>
#include <memory>

#include "frame_buffer_config.hpp"

struct PixelColor {
	std::uint8_t r;
	std::uint8_t g;
	std::uint8_t b;

	bool operator==(const PixelColor& other) const {
		return r == other.r && g == other.g && b == other.b;
	}

	bool operator!=(const PixelColor& other) const {
		return !(*this == other);
	}
};

template <typename T>
struct Vector2D {
	T x;
	T y;

	template <typename U>
	Vector2D<T>& operator+=(const Vector2D<U>& rhs) {
		x += rhs.x;
		y += rhs.y;
		return *this;
	}
};

class PixelWriter {
public:
	virtual ~PixelWriter() = default;

	virtual void write(int x, int y, const PixelColor& c) = 0;
};

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

void draw_filled_rectangle(
	PixelWriter& writer,
	const Vector2D<int>& pos,
	const Vector2D<int>& size,
	const PixelColor& c);
void draw_rectangle(PixelWriter& writer, const Vector2D<int>& pos, const Vector2D<int>& size, const PixelColor& c);
