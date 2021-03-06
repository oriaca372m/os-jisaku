#include <cstdint>
#include <cstdio>

#include <asmfunc.hpp>
#include <kernel_interface/logger.hpp>
#include <usb/classdriver/mouse.hpp>
#include <usb/device.hpp>
#include <usb/memory.hpp>
#include <usb/xhci/trb.hpp>
#include <usb/xhci/xhci.hpp>

#include "graphics/console.hpp"
#include "graphics/frame_buffer_config.hpp"
#include "graphics/graphics.hpp"
#include "graphics/layer.hpp"
#include "graphics/mouse.hpp"
#include "interrupt.hpp"
#include "logger.hpp"
#include "memory_manager.hpp"
#include "memory_map.hpp"
#include "paging.hpp"
#include "pci.hpp"
#include "queue.hpp"
#include "sbrk.hpp"
#include "segment.hpp"
#include "timer.hpp"
#include "utils.hpp"
#include "window.hpp"

namespace {
	unsigned int mouse_layer_id;
	Vector2D<int> screen_size;

	void mouse_observer(std::uint8_t buttons, std::int8_t dx, std::int8_t dy) {
		static unsigned int mouse_drag_layer_id = 0;
		static std::uint8_t previous_buttons = 0;
		static Vector2D<int> mouse_position = {0, 0};

		const auto old_pos = mouse_position;
		const auto new_pos = mouse_position + Vector2D<int>(dx, dy);
		mouse_position = new_pos.max({0, 0}).min(screen_size - Vector2D<int>(1, 1));
		layer_manager->move(mouse_layer_id, mouse_position);

		const auto pos_diff = mouse_position - old_pos;

		const bool is_previous_left_pressed = previous_buttons & 0x01;
		const bool is_left_pressed = buttons & 0x01;

		if (!is_previous_left_pressed && is_left_pressed) {
			// ドラッグを始めた瞬間
			auto layer = layer_manager->find_layer_by_position(mouse_position, mouse_layer_id);
			if (layer != nullptr) {
				mouse_drag_layer_id = layer->id();
			}
		} else if (is_previous_left_pressed && is_left_pressed) {
			// ドラッグ中
			if (mouse_drag_layer_id > 0) {
				layer_manager->move_relative(mouse_drag_layer_id, pos_diff);
			}
		} else if (is_previous_left_pressed && !is_left_pressed) {
			// ドラッグを終えた瞬間
			mouse_drag_layer_id = 0;
		}

		previous_buttons = buttons;
	}

	struct Message {
		enum class Type {
			InterruptXHCI,
		} type;
	};

	ArrayQueue<Message>* main_queue;

	__attribute__((interrupt)) void int_handler_xhci(InterruptFrame* frame) {
		main_queue->push(Message{Message::Type::InterruptXHCI});
		notify_end_of_interrput();
	}

	alignas(BitmapMemoryManager) std::uint8_t memory_manager_buf[sizeof(BitmapMemoryManager)];
	BitmapMemoryManager* memory_manager = reinterpret_cast<BitmapMemoryManager*>(&memory_manager_buf);
}

extern "C" void kernel_main(const FrameBufferConfig& frame_buffer_config_ref, const MemoryMap& memory_map_ref) {
	auto frame_buffer_config = frame_buffer_config_ref;
	auto memory_map = memory_map_ref;

	char fb_pixel_writer_buf[max_device_pixel_writer_size];
	get_suitable_device_pixel_writer_traits(frame_buffer_config.pixel_format)
		.construct(frame_buffer_config, fb_pixel_writer_buf);
	auto fb_pixel_writer = reinterpret_cast<DevicePixelWriter*>(fb_pixel_writer_buf);

	const PixelColor desktop_fg_color{0xc8, 0xc8, 0xc6};
	const PixelColor desktop_bg_color{0x1d, 0x1f, 0x21};

	const int frame_width = frame_buffer_config.horizontal_resolution;
	const int frame_height = frame_buffer_config.vertical_resolution;
	screen_size = {frame_width, frame_height};

	Console console_instance(desktop_fg_color, desktop_bg_color);
	global_console = &console_instance;

	console_instance.set_pixel_writer(fb_pixel_writer);

	auto console_logger = logger::ConsoleLogger(&console_instance, logger::LogLevel::Info);
	auto logger_proxy = logger::LoggerProxy(console_logger);
	kernel_interface::logger::default_logger = &console_logger;
	log = &logger_proxy;

	// セグメンテーションの設定
	setup_segments();
	const std::uint16_t kernel_cs = 1 << 3;
	const std::uint16_t kernel_ss = 2 << 3;
	set_ds_all(0);
	set_cs_ss(kernel_cs, kernel_ss);

	// ページングの設定
	setup_identity_page_table();

	// メモリマネージャの設定
	{
		new (memory_manager) BitmapMemoryManager();
		const auto memory_map_base = reinterpret_cast<std::uintptr_t>(memory_map.buffer);

		std::uintptr_t available_end = 0;
		for (std::uintptr_t iter = memory_map_base; iter < memory_map_base + memory_map.map_size;
			 iter += memory_map.descriptor_size) {
			auto desc = reinterpret_cast<const MemoryDescriptor*>(iter);

			if (available_end < desc->physical_start) {
				memory_manager->mark_allocated(
					FrameID(available_end / bytes_per_frame), (desc->physical_start - available_end) / bytes_per_frame);
			}

			if (is_available(static_cast<MemoryType>(desc->type))) {
				const auto physical_end = desc->physical_start + desc->number_of_pages * uefi_page_size;
				available_end = physical_end;
			} else {
				memory_manager->mark_allocated(
					FrameID(desc->physical_start / bytes_per_frame),
					desc->number_of_pages * uefi_page_size / bytes_per_frame);
			}
		}

		memory_manager->set_memory_range(FrameID(1), FrameID(available_end / bytes_per_frame));
	}

	if (auto err = initialize_heap(*memory_manager)) {
		log->error("Failed to allocate pages: %s\n", err.name());
		halt();
	}

	initialize_lapic_timer();

	std::array<Message, 32> main_queue_buffer;
	ArrayQueue<Message> main_queue_instance(main_queue_buffer);
	main_queue = &main_queue_instance;

	FrameBuffer screen(frame_buffer_config);
	layer_manager = new DoubleBufferedLayerManager(frame_buffer_config.pixel_format);

	auto bg_layer = layer_manager->new_layer<BufferLayer>(Vector2D<int>(frame_width, frame_width));
	bg_layer->move({0, 0});

	{
		auto painter = bg_layer->start_paint();

		painter.draw_filled_rectangle({{0, 0}, {frame_width, frame_height - 50}}, desktop_bg_color);
		painter.draw_filled_rectangle({{0, frame_height - 50}, {frame_width, frame_height}}, {1, 8, 17});
		painter.draw_filled_rectangle({{0, frame_height - 50}, {frame_width / 5, frame_height}}, {80, 80, 80});
		painter.draw_rectangle({{10, frame_height - 40}, {40, frame_height - 10}}, {160, 160, 160});

		painter.draw_filled_rectangle({{500, 500}, {600, 600}}, {255, 0, 0});
	}

	auto console_layer = layer_manager->new_layer<BufferLayer>(Vector2D<int>(640, 400));
	console_layer->move({200, 150});

	auto fast_console = FastConsole(desktop_fg_color, {0, 0, 123}, *console_layer);
	global_console = &fast_console;
	console_logger.set_console(global_console);

	auto mouse_layer = make_mouse_layer(*layer_manager);
	mouse_layer_id = mouse_layer->id();
	mouse_layer->move({200, 200});

	auto group_layer = layer_manager->new_layer<GroupLayer>(Vector2D<int>(500, 500));
	group_layer->move({0, 0});

	auto main_window_layer = layer_manager->new_layer<BufferLayer>(Vector2D<int>(160, 68));
	{
		auto painter = main_window_layer->start_paint();
		draw_window(painter, u8"Hello Window");
		painter.draw_string({8, 44}, u8"Chino-chan Kawaii!", {0, 0, 0});
	}
	main_window_layer->move({300, 300});

	Layer* test_layer;

	{
		auto& group_manager = group_layer->layer_manager();

		auto bg = group_manager.new_layer<BufferLayer>(Vector2D<int>(500, 500));
		const auto tc = PixelColor({0x00, 0xff, 0x00});
		group_layer->set_transparent_color(tc);
		bg->start_paint().draw_filled_rectangle({0, 0, 500, 500}, tc);

		auto test = group_manager.new_layer<BufferLayer>(Vector2D<int>(100, 100));
		test_layer = test;
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

	layer_manager->set_buffer(&screen);
	layer_manager->draw();

	printk(u8"chino chan kawaii!\n");
	printk(u8"gochuumon wa usagi desu ka?\n");

	const std::array available_memory_types{
		MemoryType::EfiBootServicesCode,
		MemoryType::EfiBootServicesData,
		MemoryType::EfiConventionalMemory,
	};

	for (auto iter = reinterpret_cast<std::uintptr_t>(memory_map.buffer);
		 iter < reinterpret_cast<std::uintptr_t>(memory_map.buffer) + memory_map.map_size;
		 iter += memory_map.descriptor_size) {
		auto desc = reinterpret_cast<MemoryDescriptor*>(iter);
		for (const auto& type : available_memory_types) {
			if (desc->type == static_cast<std::uint32_t>(type)) {
				printk(
					u8"type = %u, phys = %08lx - %08lx, pages = %lu, attr = %08lx\n",
					desc->type,
					desc->physical_start,
					desc->physical_start + desc->number_of_pages * 4096 - 1,
					desc->number_of_pages,
					desc->attribute);
			}
		}
	}

	auto err = pci::scan_all_bus();
	log->debug(u8"pci::scan_all_bus(): %s\n", err.name());

	pci::Device* xhc_device = nullptr;
	for (int i = 0; i < pci::num_devices; ++i) {
		if (pci::devices[i].class_code.match(0x0cu, 0x03u, 0x30u)) {
			xhc_device = &pci::devices[i];
			if (xhc_device->vendor_id == 0x8086) {
				break;
			}
		}
	}

	if (xhc_device == nullptr) {
		log->error(u8"Could not found xHC!\n");
		halt();
	}

	log->info(u8"xHC has been found: %d.%d.%d\n", xhc_device->bus, xhc_device->device, xhc_device->function);

	// 割り込みの設定
	{
		set_idt_entry(
			idt[InterruptVector::xhci],
			make_idt_attr(DescriptorType::InterruptGate, 0),
			reinterpret_cast<std::uint64_t>(int_handler_xhci),
			get_cs());
		load_idt(sizeof(idt) - 1, reinterpret_cast<std::uintptr_t>(idt.data()));

		const std::uint8_t bsp_local_apic_id = *reinterpret_cast<const std::uint32_t*>(0xfee00020) >> 24;

		const auto err = pci::configure_msi_fixed_destination(
			*xhc_device,
			bsp_local_apic_id,
			pci::MSITriggerMode::Level,
			pci::MSIDeliveryMode::Fixed,
			InterruptVector::xhci,
			0);
		log->debug(u8"pci::configure_msi_fixed_destination(): %s\n", err.name());
	}

	const auto xhc_bar = pci::read_bar(*xhc_device, 0);
	log->debug(u8"pci::read_bar(): %s\n", xhc_bar.error.name());

	const std::uint64_t xhc_mmio_base = xhc_bar.value & ~static_cast<std::uint64_t>(0xf);
	log->debug(u8"xHC mmio_base = %08lx\n", xhc_mmio_base);

	usb::xhci::Controller xhc(xhc_mmio_base);

	{
		auto err = xhc.Initialize();
		log->debug(u8"xhc.Initialize(): %s\n", err.Name());
	}

	log->info(u8"xHC starting\n");
	xhc.Run();

	// 割り込みの開始
	__asm("sti");

	usb::HIDMouseDriver::default_observer = mouse_observer;
	for (int i = 1; i <= xhc.MaxPorts(); ++i) {
		auto port = xhc.PortAt(i);
		log->debug(u8"port %d: IsConnected=%d\n", i, port.IsConnected());

		if (port.IsConnected()) {
			if (auto err = usb::xhci::ConfigurePort(xhc, port)) {
				log->error(u8"Failed to configure port: %s at %s:%d\n", err.Name(), err.File(), err.Line());
				continue;
			}
		}
	}

	int c = 0;
	char str[128];

	while (true) {
		++c;
		std::snprintf(str, sizeof(str), u8"%010u", c);
		{
			auto painter = main_window_layer->start_paint();
			painter.draw_filled_rectangle(Rect<int>::with_size({24, 28}, {8 * 10, 16}), {0xc6c6c6});
			painter.draw_string({24, 28}, str, {0x000000});
		}
		test_layer->move({10, c % 100});

		__asm__("cli");
		if (main_queue->count() == 0) {
			__asm__("sti;"
					//"hlt;"
			);
			continue;
		}

		const auto msg = main_queue->front();
		main_queue->pop();
		__asm("sti");

		switch (msg.type) {
		case Message::Type::InterruptXHCI:
			while (xhc.PrimaryEventRing()->HasFront()) {
				if (auto err = ProcessEvent(xhc)) {
					log->error(u8"Error while ProcessEvent: %s at %s:%d\n", err.Name(), err.File(), err.Line());
				}
			}

			break;
		default:
			log->error(u8"Unknown message type: %d\n", static_cast<int>(msg.type));
		}
	}
}
