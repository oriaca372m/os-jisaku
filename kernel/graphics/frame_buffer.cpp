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
	return copy_from(
		src,
		static_cast<Vector2D<std::uint32_t>>(to_pos),
		{0, 0},
		{std::min(config_.horizontal_resolution, src.config_.horizontal_resolution),
		 std::min(config_.vertical_resolution, src.config_.vertical_resolution)});
}

Error FrameBuffer::copy_from(
	const FrameBuffer& src,
	Vector2D<std::uint32_t> to_pos,
	Vector2D<std::uint32_t> src_pos,
	Vector2D<std::uint32_t> src_size) {
	if (src.config_.pixel_format != config_.pixel_format) {
		return Error::Code::UnknownPixelFormat;
	}

	const auto bpp = writer_traits_->bytes_per_pixel;

	auto* dst_buf = config_.frame_buffer + bpp * (config_.pixels_per_scan_line * to_pos.y + to_pos.x);
	const auto* src_buf = src.config_.frame_buffer + bpp * (src.config_.pixels_per_scan_line * src_pos.y + src_pos.x);

	for (int y = 0; y < src_size.y; ++y) {
		std::memcpy(dst_buf, src_buf, bpp * (src_size.x));
		dst_buf += bpp * config_.pixels_per_scan_line;
		src_buf += bpp * src.config_.pixels_per_scan_line;
	}

	return Error::Code::Success;
}
