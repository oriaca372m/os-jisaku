#include <cstdint>
#include <cstdio>

#include <asmfunc.hpp>
#include <kernel_interface/logger.hpp>
#include <usb/classdriver/mouse.hpp>
#include <usb/device.hpp>
#include <usb/memory.hpp>
#include <usb/xhci/trb.hpp>
#include <usb/xhci/xhci.hpp>

#include "console.hpp"
#include "font.hpp"
#include "frame_buffer_config.hpp"
#include "graphics.hpp"
#include "interrupt.hpp"
#include "logger.hpp"
#include "mouse.hpp"
#include "pci.hpp"
#include "queue.hpp"
#include "utils.hpp"

namespace {
	MouseCursor* mouse_cursor;

	void mouse_observer(int8_t displacement_x, int8_t displacement_y) {
		mouse_cursor->move_relative({displacement_x, displacement_y});
	}

	alignas(usb::xhci::Controller) std::uint8_t xhc_buffer[sizeof(usb::xhci::Controller)];
	usb::xhci::Controller* xhc = reinterpret_cast<usb::xhci::Controller*>(xhc_buffer);

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
}

extern "C" void KernelMain(const FrameBufferConfig& frame_buffer_config) {
	char pixel_writer_buf[sizeof(RGBResv8BitPerColorPixelWriter)];
	PixelWriter* pixel_writer = reinterpret_cast<PixelWriter*>(pixel_writer_buf);

	if (frame_buffer_config.pixel_format == PixelFormat::RGBResv8BitPerColor) {
		new (pixel_writer_buf) RGBResv8BitPerColorPixelWriter(frame_buffer_config);
	} else if (frame_buffer_config.pixel_format == PixelFormat::BGRResv8BitPerColor) {
		new (pixel_writer_buf) BGRResv8BitPerColorPixelWriter(frame_buffer_config);
	}

	const PixelColor desktop_fg_color{0xc8, 0xc8, 0xc6};
	const PixelColor desktop_bg_color{0x1d, 0x1f, 0x21};
	Console console_instance(*pixel_writer, desktop_fg_color, desktop_bg_color);
	global_console = &console_instance;

	auto console_logger = logger::ConsoleLogger(console_instance, logger::LogLevel::Info);
	auto logger_proxy = logger::LoggerProxy(console_logger);
	kernel_interface::logger::default_logger = &console_logger;
	log = &logger_proxy;

	MouseCursor mouse_cursor_instance(pixel_writer, desktop_bg_color, {200, 100});
	mouse_cursor = &mouse_cursor_instance;

	std::array<Message, 32> main_queue_buffer;
	ArrayQueue<Message> main_queue_instance(main_queue_buffer);
	main_queue = &main_queue_instance;

	const int frame_width = frame_buffer_config.horizontal_resolution;
	const int frame_height = frame_buffer_config.vertical_resolution;

	draw_filled_rectangle(*pixel_writer, {0, 0}, {frame_width, frame_height - 50}, desktop_bg_color);
	draw_filled_rectangle(*pixel_writer, {0, frame_height - 50}, {frame_width, 50}, {1, 8, 17});
	draw_filled_rectangle(*pixel_writer, {0, frame_height - 50}, {frame_width / 5, 50}, {80, 80, 80});
	draw_rectangle(*pixel_writer, {10, frame_height - 40}, {30, 30}, {160, 160, 160});

	printk(u8"chino chan kawaii!\n");
	printk(u8"gochuumon wa usagi desu ka?\n");

	auto err = pci::scan_all_bus();
	log->debug(u8"scan_all_bus: %s\n", err.name());

	pci::Device* xhc_device = nullptr;
	for (int i = 0; i < pci::num_devices; ++i) {
		if (pci::devices[i].class_code.match(0x0cu, 0x03u, 0x30u)) {
			xhc_device = &pci::devices[i];
			if (xhc_device->vendor_id == 0x8086) {
				break;
			}
		}
	}

	if (xhc_device != nullptr) {
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
			log->debug(u8"pci::configure_msi_fixed_destination: %s\n", err.name());
		}

		const auto xhc_bar = pci::read_bar(*xhc_device, 0);
		log->debug(u8"read_bar: %s\n", xhc_bar.error.name());

		const std::uint64_t xhc_mmio_base = xhc_bar.value & ~static_cast<std::uint64_t>(0xf);
		log->debug(u8"xHC mmio_base = %08lx\n", xhc_mmio_base);

		xhc = new (xhc_buffer) usb::xhci::Controller(xhc_mmio_base);

		{
			auto err = xhc->Initialize();
			log->debug(u8"xhc.Initialize(): %s\n", err.Name());
		}

		log->info(u8"xHC starting\n");
		xhc->Run();

		// 割り込みの開始
		__asm("sti");

		usb::HIDMouseDriver::default_observer = mouse_observer;
		for (int i = 1; i <= xhc->MaxPorts(); ++i) {
			auto port = xhc->PortAt(i);
			log->debug(u8"port %d: IsConnected=%d\n", i, port.IsConnected());

			if (port.IsConnected()) {
				if (auto err = usb::xhci::ConfigurePort(*xhc, port)) {
					log->error(u8"Failed to configure port: %s at %s:%d\n", err.Name(), err.File(), err.Line());
					continue;
				}
			}
		}
	}

	while (true) {
		__asm__("cli");
		if (main_queue->count() == 0) {
			__asm__(
				"sti;"
				"hlt;");
			continue;
		}

		const auto msg = main_queue->front();
		main_queue->pop();
		__asm("sti");

		switch (msg.type) {
		case Message::Type::InterruptXHCI:
			while (xhc->PrimaryEventRing()->HasFront()) {
				if (auto err = ProcessEvent(*xhc)) {
					log->error(u8"Error while ProcessEvent: %s at %s:%d\n", err.Name(), err.File(), err.Line());
				}
			}

			break;
		default:
			log->error(u8"Unknown message type: %d\n", static_cast<int>(msg.type));
		}
	}
}
