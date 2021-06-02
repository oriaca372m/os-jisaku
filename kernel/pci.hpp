#pragma once

#include <array>
#include <cstdint>

#include "error.hpp"

namespace pci {
	const std::uint16_t config_address = 0x0cf8;
	const std::uint16_t config_data = 0x0cfc;

	std::uint16_t read_vendor_id(std::uint8_t bus, std::uint8_t device, std::uint8_t function);
	std::uint32_t read_class_code(std::uint8_t bus, std::uint8_t device, std::uint8_t function);

	struct Device {
		std::uint8_t bus;
		std::uint8_t device;
		std::uint8_t function;
		std::uint8_t header_type;
	};

	inline std::array<Device, 32> devices;
	inline int num_devices;

	Error scan_all_bus();
	Error scan_bus(std::uint8_t bus);
}
