#include "frame_buffer.hpp"

#include <cstring>

FrameBuffer::FrameBuffer(const FrameBufferConfig& config) : config_(config) {
	writer_traits_ = &get_suitable_device_pixel_writer_traits(config_.pixel_format);
	const auto bpp = writer_traits_->bytes_per_pixel;

	if (config_.frame_buffer == nullptr) {
		buffer_.resize(bpp * config_.horizontal_resolution * config_.vertical_resolution);
		config_.frame_buffer = buffer_.data();
		config_.pixels_per_scan_line = config_.horizontal_resolution;
	}

	writer_ = writer_traits_->construct(config_);
}

Vector2D<int> FrameBuffer::size() const {
	return {static_cast<int>(config_.horizontal_resolution), static_cast<int>(config_.vertical_resolution)};
}

Error FrameBuffer::copy_from(const FrameBuffer& src, Vector2D<int> to_pos) {
	if (src.config_.pixel_format != config_.pixel_format) {
		return Error::Code::UnknownPixelFormat;
	}

	const auto dst_width = config_.horizontal_resolution;
	const auto dst_height = config_.vertical_resolution;
	const auto src_width = src.config_.horizontal_resolution;
	const auto src_height = src.config_.vertical_resolution;

	const int copy_start_dst_x = std::max(to_pos.x, 0);
	const int copy_start_dst_y = std::max(to_pos.y, 0);
	const int copy_end_dst_x = std::min(to_pos.x + src_width, dst_width);
	const int copy_end_dst_y = std::min(to_pos.y + src_height, dst_height);

	const auto bpp = writer_traits_->bytes_per_pixel;

	auto* dst_buf = config_.frame_buffer + bpp * (config_.pixels_per_scan_line * copy_start_dst_y + copy_start_dst_x);
	const auto* src_buf = src.config_.frame_buffer;

	for (int y = copy_start_dst_y; y < copy_end_dst_y; ++y) {
		std::memcpy(dst_buf, src_buf, bpp * (copy_end_dst_x - copy_start_dst_x));
		dst_buf += bpp * config_.pixels_per_scan_line;
		src_buf += bpp * src.config_.pixels_per_scan_line;
	}

	return Error::Code::Success;
}
