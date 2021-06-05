#include <cstdint>
#include <cstdio>

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
#include "logger.hpp"
#include "pci.hpp"
#include "utils.hpp"

namespace {
	const int mouse_cursor_width = 15;
	const int mouse_cursor_height = 24;
	const char mouse_cursor_shape[mouse_cursor_height][mouse_cursor_width + 1] = {
		"@              ", // keep shape
		"@@             ", // keep shape
		"@.@            ", // keep shape
		"@..@           ", // keep shape
		"@...@          ", // keep shape
		"@....@         ", // keep shape
		"@.....@        ", // keep shape
		"@......@       ", // keep shape
		"@.......@      ", // keep shape
		"@........@     ", // keep shape
		"@.........@    ", // keep shape
		"@..........@   ", // keep shape
		"@...........@  ", // keep shape
		"@............@ ", // keep shape
		"@......@@@@@@@@", // keep shape
		"@......@       ", // keep shape
		"@....@@.@      ", // keep shape
		"@...@ @.@      ", // keep shape
		"@..@   @.@     ", // keep shape
		"@.@    @.@     ", // keep shape
		"@@      @.@    ", // keep shape
		"@       @.@    ", // keep shape
		"         @.@   ", // keep shape
		"         @@@   ", // keep shape
	};

	void draw_mouse(PixelWriter& writer, int x, int y) {
		for (int dy = 0; dy < mouse_cursor_height; ++dy) {
			for (int dx = 0; dx < mouse_cursor_width; ++dx) {
				auto c = mouse_cursor_shape[dy][dx];
				if (c == '@') {
					writer.write(x + dx, y + dy, {0x00, 0x00, 0x00});
				} else if (c == '.') {
					writer.write(x + dx, y + dy, {0xFF, 0xFF, 0xFF});
				}
			}
		}
	}
}

void mouse_observer(int8_t displacement_x, int8_t displacement_y) {
	printk("mouse: x: %d, y: %d\n", displacement_x, displacement_y);
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

	const int frame_width = frame_buffer_config.horizontal_resolution;
	const int frame_height = frame_buffer_config.vertical_resolution;

	draw_filled_rectangle(*pixel_writer, {0, 0}, {frame_width, frame_height - 50}, desktop_bg_color);
	draw_filled_rectangle(*pixel_writer, {0, frame_height - 50}, {frame_width, 50}, {1, 8, 17});
	draw_filled_rectangle(*pixel_writer, {0, frame_height - 50}, {frame_width / 5, 50}, {80, 80, 80});
	draw_rectangle(*pixel_writer, {10, frame_height - 40}, {30, 30}, {160, 160, 160});

	printk(u8"chino chan kawaii!\n");
	printk(u8"gochuumon wa usagi desu ka?\n");

	auto err = pci::scan_all_bus();
	log->debug("scan_all_bus: %s\n", err.name());

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
		log->info("xHC has been found: %d.%d.%d\n", xhc_device->bus, xhc_device->device, xhc_device->function);

		const auto xhc_bar = pci::read_bar(*xhc_device, 0);
		const std::uint64_t xhc_mmio_base = xhc_bar & ~static_cast<std::uint64_t>(0xf);
		log->debug("xHC mmio_base = %08lx\n", xhc_mmio_base);

		usb::xhci::Controller xhc(xhc_mmio_base);

		{
			auto err = xhc.Initialize();
			log->debug("xhc.Initialize(): %s\n", err.Name());
		}

		log->info("xHC starting\n");
		xhc.Run();

		usb::HIDMouseDriver::default_observer = mouse_observer;
		for (int i = 1; i <= xhc.MaxPorts(); ++i) {
			auto port = xhc.PortAt(i);
			log->debug("port %d: IsConnected=%d\n", i, port.IsConnected());

			if (port.IsConnected()) {
				if (auto err = usb::xhci::ConfigurePort(xhc, port)) {
					log->error("Failed to configure port: %s at %s:%d\n", err.Name(), err.File(), err.Line());
					continue;
				}
			}
		}

		while (true) {
			if (auto err = usb::xhci::ProcessEvent(xhc)) {
				log->error("Error while ProcessEvent: %s at %s:%d\n", err.Name(), err.File(), err.Line());
			}
		}
	}

	draw_mouse(*pixel_writer, 200, 100);

	while (true) {
		__asm("hlt");
	}
}
