#pragma once

#include <array>
#include <cstdint>

#include "error.hpp"

namespace pci {
	const std::uint16_t config_address = 0x0cf8;
	const std::uint16_t config_data = 0x0cfc;

	struct ClassCode {
		std::uint8_t base;
		std::uint8_t sub;
		std::uint8_t interface;

		bool match(std::uint8_t b) {
			return b == base;
		}

		bool match(std::uint8_t b, std::uint8_t s) {
			return match(b) && s == sub;
		}

		bool match(std::uint8_t b, std::uint8_t s, std::uint8_t i) {
			return match(b, s) && i == interface;
		}
	};

	struct Device {
		std::uint8_t bus;
		std::uint8_t device;
		std::uint8_t function;
		std::uint8_t header_type;

		std::uint16_t vendor_id;
		ClassCode class_code;
	};

	std::uint16_t read_vendor_id(std::uint8_t bus, std::uint8_t device, std::uint8_t function);
	ClassCode read_class_code(std::uint8_t bus, std::uint8_t device, std::uint8_t function);

	WithError<std::uint64_t> read_bar(Device& device, unsigned int bar_index);

	inline std::array<Device, 32> devices;
	inline int num_devices;

	Error scan_all_bus();
	Error scan_bus(std::uint8_t bus);

	enum class MSITriggerMode { Edge = 0, Level = 1 };

	enum class MSIDeliveryMode {
		Fixed = 0b000,
		LowestPriority = 0b001,
		SMI = 0b010,
		NMI = 0b100,
		INIT = 0b101,
		ExtINT = 0b111
	};

	Error configure_msi_fixed_destination(
		const Device& device,
		std::uint8_t apic_id,
		MSITriggerMode trigger_mode,
		MSIDeliveryMode delivery_mode,
		std::uint8_t vector,
		unsigned int num_vector_exponent);

	Device* find_xhc_device();
}
