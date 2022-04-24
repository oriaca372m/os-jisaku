#pragma once

#include <memory>
#include <optional>
#include <vector>

#include "device_pixel_writer.hpp"
#include "error.hpp"
#include "frame_buffer_config.hpp"

class FrameBuffer final {
public:
	FrameBuffer(const FrameBufferConfig& config);

	Vector2D<int> size() const;

	DevicePixelWriter& writer() {
		return *writer_;
	}

	const DevicePixelWriter& writer() const {
		return *writer_;
	}

	// バッファを完全にコピーする
	void forward(const FrameBuffer& src);

	void copy_self_y(int dst_y, int src_y, int src_end_y);

	Error copy_from(const FrameBuffer& src, Vector2D<int> to_pos, std::optional<PixelColor> transparent_color);
	Error copy_from(
		const FrameBuffer& src,
		Vector2D<int> to_pos,
		Vector2D<int> src_pos,
		Vector2D<int> size,
		std::optional<PixelColor> transparent_color);

	FrameBuffer clone() const;

private:
	FrameBufferConfig config_;
	std::vector<std::uint8_t> buffer_{};
	std::unique_ptr<DevicePixelWriter> writer_{};

	const DevicePixelWriterTraits* writer_traits_;

	FrameBuffer(
		FrameBufferConfig&& config,
		std::vector<std::uint8_t>&& buffer,
		const DevicePixelWriterTraits* writer_traits);

	std::size_t buffer_size() const;
	std::size_t bytes_per_scan_line() const;
	std::uint8_t* frame_addr_at(Vector2D<int> pos) const;
};
