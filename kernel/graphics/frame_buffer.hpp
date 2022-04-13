#pragma once

#include <memory>
#include <vector>

#include "../error.hpp"
#include "device_pixel_writer.hpp"
#include "frame_buffer_config.hpp"

class FrameBuffer final {
public:
	FrameBuffer(const FrameBufferConfig& config);

	DevicePixelWriter& writer() {
		return *writer_;
	}

	Error copy_from(const FrameBuffer& src, Vector2D<int> to_pos);

private:
	FrameBufferConfig config_;
	std::vector<std::uint8_t> buffer_{};
	std::unique_ptr<DevicePixelWriter> writer_{};

	const DevicePixelWriterTraits* writer_traits_;
};
