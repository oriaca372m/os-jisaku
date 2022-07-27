#include "graphics.hpp"

#include "console.hpp"
#include "frame_buffer.hpp"
#include "layer.hpp"
#include "layer_manager.hpp"
#include "logger.hpp"
#include "mouse.hpp"
#include "utils.hpp"
#include "window.hpp"

void graphics::draw_filled_rectangle(
	PixelWriter& writer,
	const Vector2D<int>& pos,
	const Vector2D<int>& size,
	const PixelColor& c) {
	for (int dy = 0; dy < size.y; ++dy) {
		for (int dx = 0; dx < size.x; ++dx) {
			writer.write(pos.x + dx, pos.y + dy, c);
		}
	}
}

void graphics::draw_rectangle(
	PixelWriter& writer,
	const Vector2D<int>& pos,
	const Vector2D<int>& size,
	const PixelColor& c) {
	for (int dx = 0; dx < size.x; ++dx) {
		writer.write(pos.x + dx, pos.y, c);
		writer.write(pos.x + dx, pos.y + size.y - 1, c);
	}

	for (int dy = 0; dy < size.y; ++dy) {
		writer.write(pos.x, pos.y + dy, c);
		writer.write(pos.x + size.x - 1, pos.y + dy, c);
	}
}

void graphics::initialize_graphics(
	const FrameBufferConfig& frame_buffer_config,
	logger::ConsoleLogger& console_logger) {
	layer_manager = new DoubleBufferedLayerManager(frame_buffer_config.pixel_format);

	auto bg_layer = layer_manager->new_layer<BufferLayer>(screen_size);
	bg_layer->move({0, 0});

	{
		auto painter = bg_layer->start_paint();

		painter.draw_filled_rectangle({{0, 0}, {screen_size.x, screen_size.y - 50}}, desktop_bg_color);
		painter.draw_filled_rectangle({{0, screen_size.y - 50}, {screen_size.x, screen_size.y}}, {1, 8, 17});
		painter.draw_filled_rectangle({{0, screen_size.y - 50}, {screen_size.x / 5, screen_size.y}}, {80, 80, 80});
		painter.draw_rectangle({{10, screen_size.y - 40}, {40, screen_size.y - 10}}, {160, 160, 160});

		painter.draw_filled_rectangle({{500, 500}, {600, 600}}, {255, 0, 0});
	}

	auto console_layer = layer_manager->new_layer<BufferLayer>(Vector2D<int>(640, 400));
	console_layer->move({200, 150});

	auto fast_console = new FastConsole(desktop_fg_color, {0, 0, 123}, *console_layer);
	global_console = fast_console;
	console_logger.set_console(global_console);

	auto mouse_layer = make_mouse_layer(*layer_manager);
	mouse_layer_id = mouse_layer->id();
	mouse_layer->move({200, 200});

	auto group_layer = layer_manager->new_layer<GroupLayer>(Vector2D<int>(500, 500));
	group_layer_id = group_layer->id();
	group_layer->move({0, 0});

	auto main_window_layer = layer_manager->new_layer<BufferLayer>(Vector2D<int>(160, 68));
	main_window_layer_id = main_window_layer->id();
	{
		auto painter = main_window_layer->start_paint();
		draw_window(painter, u8"Hello Window");
		painter.draw_string({8, 44}, u8"Chino-chan Kawaii!", {0, 0, 0});
	}
	main_window_layer->move({300, 300});
	main_window_layer->set_draggable(true);

	{
		auto& group_manager = group_layer->layer_manager();

		auto bg = group_manager.new_layer<BufferLayer>(Vector2D<int>(500, 500));
		const auto tc = PixelColor({0x00, 0xff, 0x00});
		group_layer->set_transparent_color(tc);
		bg->start_paint().draw_filled_rectangle({0, 0, 500, 500}, tc);

		auto test = group_manager.new_layer<BufferLayer>(Vector2D<int>(100, 100));
		test_layer_id = test->id();
		{
			auto p = test->start_paint();
			p.draw_filled_rectangle(Rect<int>::with_size({0, 0}, {100, 100}), {0xff, 0x00, 0xff});
			p.draw_filled_rectangle(Rect<int>::with_size({30, 30}, {50, 50}), {0xff, 0xff, 0x00});
		}
		test->move({10, 10});

		group_manager.up_down(bg->id(), 0);
		group_manager.up_down(test->id(), 1);
	}

	layer_manager->up_down(bg_layer->id(), 0);
	layer_manager->up_down(console_layer->id(), 1);
	layer_manager->up_down(group_layer->id(), 2);
	layer_manager->up_down(main_window_layer->id(), 3);
	layer_manager->up_down(mouse_layer->id(), 4);

	auto* screen = new FrameBuffer(frame_buffer_config);
	layer_manager->set_buffer(screen);
	layer_manager->draw();

	printk(u8"chino chan kawaii!\n");
	printk(u8"gochuumon wa usagi desu ka?\n");
}
