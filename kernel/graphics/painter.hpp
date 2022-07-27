#pragma once

#include "frame_buffer.hpp"

namespace graphics {
	class Layer;

	class Painter final {
	public:
		Painter(FrameBuffer& buffer, Layer& layer);
		~Painter();

		void end();

		Vector2D<int> size() const;

		void copy_y(int dst_y, int src_y, int src_end_y);

		void draw_rectangle(const Rect<int>& rect, const PixelColor& c);
		void draw_filled_rectangle(const Rect<int>& rect, const PixelColor& c);
		void draw_ascii(const Vector2D<int>& pos, char ch, const PixelColor& c);
		void draw_string(const Vector2D<int>& pos, const char* str, const PixelColor& c);
		PixelWriter& pixel_writer();

		PixelWriter& raw_pixel_writer();
		FrameBuffer& raw_buffer();
		void raw_damage(const Rect<int>& rect);

	private:
		FrameBuffer& buffer_;
		Layer& layer_;

		bool damaged_ = false;
		Vector2D<int> damage_top_left_;
		Vector2D<int> damage_bottom_right_;
	};
}
