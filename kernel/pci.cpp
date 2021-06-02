#include "pci.hpp"
#include "asmfunc.hpp"
#include "utils.hpp"

namespace pci {
	std::uint32_t make_address(std::uint8_t bus, std::uint8_t device, std::uint8_t function, std::uint8_t reg_addr) {
		auto shl = [](std::uint32_t x, unsigned int bits) { return x << bits; };

		return shl(1, 31) | shl(bus, 16) | shl(device, 11) | shl(function, 8) | (reg_addr & 0b1111'1100u);
	}

	void write_address(std::uint32_t address) {
		io_out_32(config_address, address);
	}

	void write_data(std::uint32_t value) {
		io_out_32(config_data, value);
	}

	std::uint32_t read_data() {
		return io_in_32(config_data);
	}

	std::uint16_t read_vendor_id(std::uint8_t bus, std::uint8_t device, std::uint8_t function) {
		write_address(make_address(bus, device, function, 0x00));
		return read_data() & 0xffffu;
	}

	std::uint8_t read_header_type(std::uint8_t bus, std::uint8_t device, std::uint8_t function) {
		write_address(make_address(bus, device, function, 0x0c));
		return (read_data() >> 16) & 0xffu;
	}

	std::uint32_t read_class_code(std::uint8_t bus, std::uint8_t device, std::uint8_t function) {
		write_address(make_address(bus, device, function, 0x08));
		return read_data();
	}

	std::uint32_t read_bus_numbers(std::uint8_t bus, std::uint8_t device, std::uint8_t function) {
		write_address(make_address(bus, device, function, 0x18));
		return read_data();
	}

	bool is_single_function_device(std::uint8_t header_type) {
		return (header_type & 0b1000'0000u) == 0;
	}

	Error add_device(std::uint8_t bus, std::uint8_t device, std::uint8_t function, std::uint8_t header_type) {
		if (num_devices == devices.size()) {
			return Error::Code::Full;
		}

		devices[num_devices] = Device{bus, device, function, header_type};
		++num_devices;
		return Error::Code::Success;
	}

	Error scan_function(std::uint8_t bus, std::uint8_t device, std::uint8_t function) {
		auto header_type = read_header_type(bus, device, function);
		if (auto err = add_device(bus, device, function, header_type)) {
			return err;
		}

		auto class_code = read_class_code(bus, device, function);
		std::uint8_t base = (class_code >> 24) & 0xffu;
		std::uint8_t sub = (class_code >> 16) & 0xffu;

		if (base == 0x06u && sub == 0x04u) {
			auto bus_numbers = read_bus_numbers(bus, device, function);
			std::uint8_t secondary_bus = (bus_numbers >> 8) & 0xffu;
			return scan_bus(secondary_bus);
		}

		return Error::Code::Success;
	}

	Error scan_device(std::uint8_t bus, std::uint8_t device) {
		if (auto err = scan_function(bus, device, 0)) {
			return err;
		}

		if (is_single_function_device(read_header_type(bus, device, 0))) {
			return Error::Code::Success;
		}

		for (std::uint8_t function = 1; function < 8; ++function) {
			if (read_vendor_id(bus, device, function) == 0xffffu) {
				continue;
			}

			if (auto err = scan_function(bus, device, function)) {
				return err;
			}
		}

		return Error::Code::Success;
	}

	Error scan_bus(std::uint8_t bus) {
		for (std::uint8_t device = 0; device < 32; ++device) {
			if (read_vendor_id(bus, device, 0) == 0xffffu) {
				continue;
			}

			if (auto err = scan_device(bus, device)) {
				return err;
			}
		}

		return Error::Code::Success;
	}

	Error scan_all_bus() {
		num_devices = 0;

		auto header_type = read_header_type(0, 0, 0);
		if (is_single_function_device(header_type)) {
			return scan_bus(0);
		}

		for (std::uint8_t function = 1; function < 8; ++function) {
			if (read_vendor_id(0, 0, function) == 0xffffu) {
				continue;
			}

			if (auto err = scan_bus(function)) {
				return err;
			}
		}

		return Error::Code::Success;
	}
}
