#include "frame_buffer.hpp"

#include <cstring>

namespace {
	void adjust_coords(
		const Vector2D<std::uint32_t>& to_size,
		const Vector2D<std::uint32_t>& src_size,
		Vector2D<int>& to_pos,
		Vector2D<int>& src_pos,
		Vector2D<int>& size) {
		// dの符号が負の時の話なのでv+dをすると小さくなることに注意
		const auto decrease_d = [](int v, int d) { return d < 0 ? v + d : v; };
		const auto increase_d = [](int v, int d) { return d < 0 ? v - d : v; };

		// src_posが負の時の座標調整
		// src_posが負ならその分sizeを縮め、to_posを増やし、src_posを0にする
		size = size.map(decrease_d, src_pos);
		to_pos = to_pos.map(increase_d, src_pos);
		src_pos = src_pos.max({0, 0});

		// to_posが負の時の座標調整
		// to_posが負ならその分sizeを縮め、to_posを増やし、to_posを0にする
		size = size.map(decrease_d, to_pos);
		src_pos = src_pos.map(increase_d, to_pos);
		to_pos = to_pos.max({0, 0});

		// sizeがto/srcの大きさを超えないように
		size = size.map(
			[](int size, std::uint32_t to_size, int to_pos, std::uint32_t src_res, int src_size) {
				const auto max_size =
					std::min(static_cast<int>(to_size) - to_pos, static_cast<int>(src_res) - src_size);
				return std::min(size, max_size);
			},
			to_size,
			to_pos,
			src_size,
			src_pos);
	}

	void copy_transparent(
		std::uint8_t* dst_buf,
		const std::uint8_t* src_buf,
		std::uint8_t* tc_buf,
		const Vector2D<int>& size,
		std::size_t bpp,
		std::size_t dst_bpl,
		std::size_t src_bpl) {
		for (int y = 0; y < size.y; ++y) {
			auto dst_buf_x = dst_buf;
			auto src_buf_x = src_buf;

			for (int x = 0; x < size.x; ++x) {
				if (std::memcmp(src_buf_x, tc_buf, bpp) != 0) {
					std::memcpy(dst_buf_x, src_buf_x, bpp);
				}

				dst_buf_x += bpp;
				src_buf_x += bpp;
			}

			dst_buf += dst_bpl;
			src_buf += src_bpl;
		}
	}

	// 適当ベンチマークによると大体10倍速い
	// 透明色見ないのと大体同じ速度出る
	void copy_transparent_bpp_4(
		std::uint8_t* dst_buf,
		const std::uint8_t* src_buf,
		std::uint8_t* tc_buf,
		const Vector2D<int>& size,
		std::size_t dst_bpl,
		std::size_t src_bpl) {
		const auto tc = *reinterpret_cast<std::uint32_t*>(tc_buf);

		for (int y = 0; y < size.y; ++y) {
			auto dst_buf_x = dst_buf;
			auto src_buf_x = src_buf;

			for (int x = 0; x < size.x; ++x) {
				const auto src_px = reinterpret_cast<const std::uint32_t*>(src_buf_x);
				if (*src_px != tc) {
					const auto dst_px = reinterpret_cast<std::uint32_t*>(dst_buf_x);
					*dst_px = *src_px;
				}

				dst_buf_x += 4;
				src_buf_x += 4;
			}

			dst_buf += dst_bpl;
			src_buf += src_bpl;
		}
	}
}

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

void FrameBuffer::copy_self_y(int dst_y, int src_y, int length) {
	std::memmove(frame_addr_at({0, dst_y}), frame_addr_at({0, src_y}), length * bytes_per_scan_line());
}

Error FrameBuffer::copy_from(
	const FrameBuffer& src,
	Vector2D<int> to_pos,
	std::optional<PixelColor> transparent_color) {
	return copy_from(
		src,
		to_pos,
		{0, 0},
		{std::min(static_cast<int>(config_.horizontal_resolution), static_cast<int>(src.config_.horizontal_resolution)),
		 std::min(static_cast<int>(config_.vertical_resolution), static_cast<int>(src.config_.vertical_resolution))},
		transparent_color);
}

Error FrameBuffer::copy_from(
	const FrameBuffer& src,
	Vector2D<int> to_pos,
	Vector2D<int> src_pos,
	Vector2D<int> size,
	std::optional<PixelColor> transparent_color) {
	if (src.config_.pixel_format != config_.pixel_format) {
		return Error::Code::UnknownPixelFormat;
	}

	adjust_coords(config_.size(), src.config_.size(), to_pos, src_pos, size);

	// 描画するものがない
	if (size.x <= 0 || size.y <= 0) {
		return Error::Code::Success;
	}

	const auto bpp = writer_traits_->bytes_per_pixel;

	auto* dst_buf = frame_addr_at(to_pos);
	const auto* src_buf = src.frame_addr_at(src_pos);

	const auto dst_bpl = bytes_per_scan_line();
	const auto src_bpl = src.bytes_per_scan_line();

	if (transparent_color) {
		// 流石に1pxに32byte以上使うピクセルフォーマットは無いと思う
		std::uint8_t tc_buf[32];
		writer_->write_to_buf(tc_buf, *transparent_color);

		if (bpp == 4) {
			copy_transparent_bpp_4(dst_buf, src_buf, tc_buf, size, dst_bpl, src_bpl);
		} else {
			copy_transparent(dst_buf, src_buf, tc_buf, size, bpp, dst_bpl, src_bpl);
		}
	} else {
		for (int y = 0; y < size.y; ++y) {
			std::memcpy(dst_buf, src_buf, bpp * size.x);
			dst_buf += dst_bpl;
			src_buf += src_bpl;
		}
	}

	return Error::Code::Success;
}

std::size_t FrameBuffer::bytes_per_scan_line() const {
	return writer_traits_->bytes_per_pixel * config_.pixels_per_scan_line;
}

std::uint8_t* FrameBuffer::frame_addr_at(Vector2D<int> pos) const {
	return config_.frame_buffer + writer_traits_->bytes_per_pixel * (config_.pixels_per_scan_line * pos.y + pos.x);
}
