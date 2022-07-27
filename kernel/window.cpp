#include "window.hpp"

namespace {
	const int close_button_width = 16;
	const int close_button_height = 14;
	const char close_button[close_button_height][close_button_width + 1] = {
		u8"...............@", // keep shape
		u8".:::::::::::::$@", // keep shape
		u8".:::::::::::::$@", // keep shape
		u8".:::@@::::@@::$@", // keep shape
		u8".::::@@::@@:::$@", // keep shape
		u8".:::::@@@@::::$@", // keep shape
		u8".::::::@@:::::$@", // keep shape
		u8".:::::@@@@::::$@", // keep shape
		u8".::::@@::@@:::$@", // keep shape
		u8".:::@@::::@@::$@", // keep shape
		u8".:::::::::::::$@", // keep shape
		u8".:::::::::::::$@", // keep shape
		u8".$$$$$$$$$$$$$$@", // keep shape
		u8"@@@@@@@@@@@@@@@@", // keep shape
	};
}

void draw_window(graphics::Painter& painter, const char* title) {
	using graphics::PixelColor;

	const auto size = painter.size();
	const auto w = size.x;
	const auto h = size.y;

	const auto c_bg = PixelColor(0xc6c6c6);
	const auto c_fg = PixelColor(0x000000);

	const auto c_highlight1 = PixelColor(0xc6c6c6);
	const auto c_highlight2 = PixelColor(0xffffff);
	const auto c_shadow1 = PixelColor(0x848484);
	const auto c_shadow2 = PixelColor(0x000000);

	const auto c_title_bg = PixelColor(0x000084);
	const auto c_title_fg = PixelColor(0xffffff);

	// ┌みたいな形のハイライト1
	painter.draw_rectangle({0, 0, w, 1}, c_highlight1);
	painter.draw_rectangle({0, 0, 1, h}, c_highlight1);

	// ┌みたいな形のハイライト2
	painter.draw_rectangle({1, 1, w - 1, 2}, c_highlight2);
	painter.draw_rectangle({1, 1, 2, h - 1}, c_highlight2);

	// ウィンドウ背景
	painter.draw_filled_rectangle({2, 2, w - 2, h - 2}, c_bg);

	// ┘みたいな形の影1
	painter.draw_filled_rectangle({1, h - 2, w - 1, h - 1}, c_shadow1);
	painter.draw_filled_rectangle({w - 2, 1, w - 1, h - 1}, c_shadow1);

	// ┘みたいな形の影2
	painter.draw_rectangle({0, h - 1, w, h}, c_shadow2);
	painter.draw_rectangle({w - 1, 0, w, h}, c_shadow2);

	// タイトルバー
	painter.draw_filled_rectangle({3, 3, w - 3, 3 + 18}, c_title_bg);
	painter.draw_string({24, 4}, title, c_title_fg);

	auto& writer = painter.raw_pixel_writer();

	// ☓ボタン
	for (int y = 0; y < close_button_height; ++y) {
		for (int x = 0; x < close_button_width; ++x) {
			PixelColor c = c_bg;
			switch (close_button[y][x]) {
			case u8'@':
				c = c_fg;
				break;
			case u8'.':
				c = c_highlight2;
				break;
			case u8'$':
				c = c_shadow1;
				break;
			}

			writer.write(w - 5 - close_button_width + x, 5 + y, c);
		}
	}
}
