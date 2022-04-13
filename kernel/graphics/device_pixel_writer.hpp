#pragma once

#include "graphics.hpp"
#include <algorithm>
#include <memory>

class DevicePixelWriter : public PixelWriter {
public:
	DevicePixelWriter(const FrameBufferConfig& config) : config_{config} {};

protected:
	const FrameBufferConfig config_;
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

class RGBResv8BitPerColorPixelWriter final : public DevicePixelWriter {
public:
	using DevicePixelWriter::DevicePixelWriter;

	void write(int x, int y, const PixelColor& c) override;

	static constexpr std::size_t bytes_per_pixel = 4;
};

struct RGBResv8BitPerColorPixelWriterTraits final : public DevicePixelWriterTraitsBase<RGBResv8BitPerColorPixelWriter> {
	static const RGBResv8BitPerColorPixelWriterTraits instance;
};

class BGRResv8BitPerColorPixelWriter final : public DevicePixelWriter {
public:
	using DevicePixelWriter::DevicePixelWriter;

	void write(int x, int y, const PixelColor& c) override;

	static constexpr std::size_t bytes_per_pixel = 4;
};

struct BGRResv8BitPerColorPixelWriterTraits final : public DevicePixelWriterTraitsBase<BGRResv8BitPerColorPixelWriter> {
	static const BGRResv8BitPerColorPixelWriterTraits instance;
};

inline constexpr std::size_t max_device_pixel_writer_size =
	std::max(sizeof(RGBResv8BitPerColorPixelWriter), sizeof(BGRResv8BitPerColorPixelWriter));
const DevicePixelWriterTraits& get_suitable_device_pixel_writer_traits(PixelFormat pixel_format);
