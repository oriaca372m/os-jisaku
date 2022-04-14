#pragma once

#include <memory>
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

	Error copy_from(const FrameBuffer& src, Vector2D<int> to_pos);
	Error copy_from(
		const FrameBuffer& src,
		Vector2D<std::uint32_t> to_pos,
		Vector2D<std::uint32_t> src_pos,
		Vector2D<std::uint32_t> src_size);

private:
	FrameBufferConfig config_;
	std::vector<std::uint8_t> buffer_{};
	std::unique_ptr<DevicePixelWriter> writer_{};

	const DevicePixelWriterTraits* writer_traits_;
};
