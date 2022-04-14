#pragma once

#include <algorithm>
#include <memory>

#include "graphics.hpp"

class DevicePixelWriter : public PixelWriter {
public:
	DevicePixelWriter(const FrameBufferConfig& config) : config_{config} {};

	void write(int x, int y, const PixelColor& c) override final {
		write_to_buf(get_pixel_buf_at(x, y), c);
	};

	virtual void write_to_buf(std::uint8_t* buf, const PixelColor& c) = 0;
	virtual void write_pixel_buf_at(int x, int y, const std::uint8_t* buf) = 0;

	virtual std::uint8_t* get_pixel_buf_at(int x, int y) const = 0;
	virtual PixelColor get_pixel_at(int x, int y) const = 0;

protected:
	const FrameBufferConfig config_;
};

template <std::size_t bpp>
class DevicePixelWriterBase : public DevicePixelWriter {
public:
	using DevicePixelWriter::DevicePixelWriter;

	void write_pixel_buf_at(int x, int y, const std::uint8_t* buf) override final {
		std::memcpy(get_pixel_buf_at(x, y), buf, bytes_per_pixel);
	}

	std::uint8_t* get_pixel_buf_at(int x, int y) const override final {
		const int pixel_position = config_.pixels_per_scan_line * y + x;
		return config_.frame_buffer + bytes_per_pixel * pixel_position;
	}

	static constexpr std::size_t bytes_per_pixel = bpp;

protected:
	std::uint8_t* get_pixel_buf_at(int x, int y) {
		const int pixel_position = config_.pixels_per_scan_line * y + x;
		return config_.frame_buffer + bytes_per_pixel * pixel_position;
	}
};

struct DevicePixelWriterTraits {
	DevicePixelWriterTraits(const int bpp) : bytes_per_pixel(bpp) {}

	virtual std::unique_ptr<DevicePixelWriter> construct(const FrameBufferConfig& config) const = 0;
	virtual void construct(const FrameBufferConfig& config, char* buf) const = 0;

	const std::size_t bytes_per_pixel;
};

template <typename T>
struct DevicePixelWriterTraitsBase : public DevicePixelWriterTraits {
	using WriterType = T;
	DevicePixelWriterTraitsBase() : DevicePixelWriterTraits(WriterType::bytes_per_pixel){};

	std::unique_ptr<DevicePixelWriter> construct(const FrameBufferConfig& config) const override {
		return std::make_unique<WriterType>(config);
	}

	void construct(const FrameBufferConfig& config, char* buf) const override {
		new (buf) WriterType(config);
	}
};

class RGBResv8BitPerColorPixelWriter final : public DevicePixelWriterBase<4> {
public:
	using DevicePixelWriterBase::DevicePixelWriterBase;

	void write_to_buf(std::uint8_t* buf, const PixelColor& c) override;
	PixelColor get_pixel_at(int x, int y) const override;
};

struct RGBResv8BitPerColorPixelWriterTraits final : public DevicePixelWriterTraitsBase<RGBResv8BitPerColorPixelWriter> {
	static const RGBResv8BitPerColorPixelWriterTraits instance;
};

class BGRResv8BitPerColorPixelWriter final : public DevicePixelWriterBase<4> {
public:
	using DevicePixelWriterBase::DevicePixelWriterBase;

	void write_to_buf(std::uint8_t* buf, const PixelColor& c) override;
	PixelColor get_pixel_at(int x, int y) const override;
};

struct BGRResv8BitPerColorPixelWriterTraits final : public DevicePixelWriterTraitsBase<BGRResv8BitPerColorPixelWriter> {
	static const BGRResv8BitPerColorPixelWriterTraits instance;
};

inline constexpr std::size_t max_device_pixel_writer_size =
	std::max(sizeof(RGBResv8BitPerColorPixelWriter), sizeof(BGRResv8BitPerColorPixelWriter));
const DevicePixelWriterTraits& get_suitable_device_pixel_writer_traits(PixelFormat pixel_format);
