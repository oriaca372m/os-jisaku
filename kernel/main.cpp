#include <cstdint>
#include <cstdio>
#include <optional>
#include <queue>

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
#include "graphics/group_layer.hpp"
#include "graphics/layer_ids.hpp"
#include "graphics/mouse.hpp"
#include "interrupt.hpp"
#include "logger.hpp"
#include "memory_manager.hpp"
#include "paging.hpp"
#include "pci.hpp"
#include "sbrk.hpp"
#include "segment.hpp"
#include "timer.hpp"
#include "utils.hpp"

namespace {
	void mouse_observer(std::uint8_t buttons, std::int8_t dx, std::int8_t dy) {
		using graphics::layer_manager;
		namespace layer_ids = graphics::layer_ids;
		using graphics::Vector2D;

		static std::optional<graphics::LayerId> dragging_layer_id;
		static std::uint8_t previous_buttons = 0;
		static Vector2D<int> mouse_position = {0, 0};

		const auto old_pos = mouse_position;
		const auto new_pos = mouse_position + Vector2D<int>(dx, dy);
		mouse_position = new_pos.max({0, 0}).min(graphics::screen_size - Vector2D<int>(1, 1));
		layer_manager->move(layer_ids::mouse, mouse_position);

		const auto pos_diff = mouse_position - old_pos;

		const bool is_previous_left_pressed = previous_buttons & 0x01;
		const bool is_left_pressed = buttons & 0x01;

		if (!is_previous_left_pressed && is_left_pressed) {
			// ドラッグを始めた瞬間
			auto layer = layer_manager->find_layer_by_position(mouse_position, layer_ids::mouse);
			if (layer != nullptr && layer->is_draggable()) {
				dragging_layer_id = layer->id();
			}
		} else if (is_previous_left_pressed && is_left_pressed) {
			// ドラッグ中
			if (dragging_layer_id) {
				layer_manager->move_relative(*dragging_layer_id, pos_diff);
			}
		} else if (is_previous_left_pressed && !is_left_pressed) {
			// ドラッグを終えた瞬間
			dragging_layer_id.reset();
		}

		previous_buttons = buttons;
	}

	struct Message {
		enum class Type {
			InterruptXHCI,
		} type;
	};

	std::queue<Message>* main_queue;

	__attribute__((interrupt)) void int_handler_xhci(InterruptFrame* frame) {
		main_queue->push(Message{Message::Type::InterruptXHCI});
		notify_end_of_interrput();
	}

	alignas(BitmapMemoryManager) std::uint8_t memory_manager_buf[sizeof(BitmapMemoryManager)];
	BitmapMemoryManager* memory_manager = reinterpret_cast<BitmapMemoryManager*>(&memory_manager_buf);

	alignas(std::max_align_t) char fb_pixel_writer_buf[graphics::max_device_pixel_writer_size];
	graphics::DevicePixelWriter* fb_pixel_writer = reinterpret_cast<graphics::DevicePixelWriter*>(fb_pixel_writer_buf);
}

extern "C" void
kernel_main(const graphics::FrameBufferConfig& frame_buffer_config_ref, const MemoryMap& memory_map_ref) {
	auto frame_buffer_config = frame_buffer_config_ref;
	auto memory_map = memory_map_ref;

	get_suitable_device_pixel_writer_traits(frame_buffer_config.pixel_format)
		.construct(frame_buffer_config, fb_pixel_writer_buf);

	const int frame_width = frame_buffer_config.horizontal_resolution;
	const int frame_height = frame_buffer_config.vertical_resolution;
	graphics::screen_size = {frame_width, frame_height};

	graphics::Console console_instance(graphics::desktop_fg_color, graphics::desktop_bg_color);
	graphics::global_console = &console_instance;

	console_instance.set_pixel_writer(fb_pixel_writer);

	auto console_logger = logger::ConsoleLogger(&console_instance, logger::LogLevel::Info);
	auto logger_proxy = logger::LoggerProxy(console_logger);
	kernel_interface::logger::default_logger = &console_logger;
	log = &logger_proxy;

	// セグメンテーションの設定
	initialize_segmentation();
	// ページングの設定
	setup_identity_page_table();

	// メモリマネージャの設定
	new (memory_manager) BitmapMemoryManager();
	initialize_memory_manager(memory_map, *memory_manager);

	if (auto err = initialize_heap(*memory_manager)) {
		log->panic("Failed to allocate pages: %s\n", err.name());
	}

	initialize_lapic_timer();

	std::queue<Message> main_queue_instance;
	main_queue = &main_queue_instance;

	initialize_graphics(frame_buffer_config, console_logger);

	auto err = pci::scan_all_bus();
	log->debug(u8"pci::scan_all_bus(): %s\n", err.name());

	auto xhc_device = pci::find_xhc_device();
	if (xhc_device == nullptr) {
		log->panic(u8"Could not found xHC!\n");
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

	namespace layer_ids = graphics::layer_ids;
	auto main_window_layer =
		static_cast<graphics::BufferLayer*>(graphics::layer_manager->find_layer(layer_ids::main_window));
	auto group_layer = static_cast<graphics::GroupLayer*>(graphics::layer_manager->find_layer(layer_ids::group_layer));
	auto test_layer = group_layer->layer_manager().find_layer(layer_ids::test_layer);

	while (true) {
		++c;
		std::snprintf(str, sizeof(str), u8"%010u", c);
		{
			auto painter = main_window_layer->start_paint();
			painter.draw_filled_rectangle(graphics::Rect<int>::with_size({24, 28}, {8 * 10, 16}), {0xc6c6c6});
			painter.draw_string({24, 28}, str, {0x000000});
		}
		test_layer->move({10, c % 100});

		__asm__("cli");
		if (main_queue->empty()) {
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
