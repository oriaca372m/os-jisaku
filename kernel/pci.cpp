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

	ClassCode read_class_code(std::uint8_t bus, std::uint8_t device, std::uint8_t function) {
		write_address(make_address(bus, device, function, 0x08));
		auto data = read_data();

		ClassCode cc;
		cc.base = (data >> 24) & 0xffu;
		cc.sub = (data >> 16) & 0xffu;
		cc.interface = (data >> 8) & 0xffu;
		return cc;
	}

	std::uint32_t read_bus_numbers(std::uint8_t bus, std::uint8_t device, std::uint8_t function) {
		write_address(make_address(bus, device, function, 0x18));
		return read_data();
	}

	std::uint32_t read_conf_reg(const Device& device, std::uint8_t reg_addr) {
		write_address(make_address(device.bus, device.device, device.function, reg_addr));
		return read_data();
	}

	void write_conf_reg(const Device& device, std::uint8_t reg_addr, std::uint32_t value) {
		write_address(make_address(device.bus, device.device, device.function, reg_addr));
		write_data(value);
	}

	constexpr std::uint8_t calc_bar_address(unsigned int bar_index) {
		return 0x10 + 4 * bar_index;
	}

	WithError<std::uint64_t> read_bar(Device& device, unsigned int bar_index) {
		if (bar_index >= 6) {
			return {0, Error::Code::IndexOutOfRange};
		}

		const auto addr = calc_bar_address(bar_index);
		auto bar = read_conf_reg(device, addr);

		if ((bar & 4u) == 0) {
			return {bar, Error::Code::Success};
		}

		if (bar_index >= 5) {
			return {0, Error::Code::IndexOutOfRange};
		}

		const auto bar_upper = read_conf_reg(device, addr + 4);
		return {bar | (static_cast<std::uint64_t>(bar_upper) << 32), Error::Code::Success};
	}

	bool is_single_function_device(std::uint8_t header_type) {
		return (header_type & 0b1000'0000u) == 0;
	}

	Error add_device(
		std::uint8_t bus,
		std::uint8_t device,
		std::uint8_t function,
		std::uint8_t header_type,
		const ClassCode& class_code) {
		if (num_devices == devices.size()) {
			return Error::Code::Full;
		}

		auto vendor_id = read_vendor_id(bus, device, function);
		devices[num_devices] = Device{bus, device, function, header_type, vendor_id, class_code};

		++num_devices;
		return Error::Code::Success;
	}

	Error scan_function(std::uint8_t bus, std::uint8_t device, std::uint8_t function) {
		auto header_type = read_header_type(bus, device, function);
		auto class_code = read_class_code(bus, device, function);

		if (auto err = add_device(bus, device, function, header_type, class_code)) {
			return err;
		}

		if (class_code.base == 0x06u && class_code.sub == 0x04u) {
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

	union CapabilityHeader {
		std::uint32_t data;
		struct {
			std::uint32_t cap_id : 8;
			std::uint32_t next_ptr : 8;
			std::uint32_t cap : 16;
		} __attribute__((packed)) bits;
	} __attribute__((packed));

	struct MSICapability {
		union {
			std::uint32_t data;
			struct {
				std::uint32_t cap_id : 8;
				std::uint32_t next_ptr : 8;
				std::uint32_t msi_enable : 1;
				std::uint32_t multi_msg_capable : 3;
				std::uint32_t multi_msg_enable : 3;
				std::uint32_t addr_64_capable : 1;
				std::uint32_t per_vector_mask_capable : 1;
				std::uint32_t : 7;
			} __attribute__((packed)) bits;
		} __attribute__((packed)) header;

		std::uint32_t msg_addr;
		std::uint32_t msg_upper_addr;
		std::uint32_t msg_data;
		std::uint32_t mask_bits;
		std::uint32_t pending_bits;
	} __attribute__((packed));

	CapabilityHeader read_capability_header(const Device& device, std::uint8_t addr) {
		pci::CapabilityHeader header;
		header.data = pci::read_conf_reg(device, addr);
		return header;
	}

	static constexpr std::uint8_t capability_msi = 0x05;
	static constexpr std::uint8_t capability_msix = 0x11;

	MSICapability read_msi_capability(const Device& device, std::uint8_t cap_addr) {
		MSICapability msi_cap{};

		msi_cap.header.data = read_conf_reg(device, cap_addr);
		msi_cap.msg_addr = read_conf_reg(device, cap_addr + 4);

		std::uint8_t msg_data_addr = cap_addr + 8;
		if (msi_cap.header.bits.addr_64_capable) {
			msi_cap.msg_upper_addr = read_conf_reg(device, cap_addr + 8);
			msg_data_addr = cap_addr + 12;
		}

		msi_cap.msg_data = read_conf_reg(device, msg_data_addr);

		if (msi_cap.header.bits.per_vector_mask_capable) {
			msi_cap.mask_bits = read_conf_reg(device, msg_data_addr + 4);
			msi_cap.pending_bits = read_conf_reg(device, msg_data_addr + 8);
		}

		return msi_cap;
	}

	void write_msi_capability(const Device& device, std::uint8_t cap_addr, const MSICapability& msi_cap) {
		write_conf_reg(device, cap_addr, msi_cap.header.data);
		write_conf_reg(device, cap_addr + 4, msi_cap.msg_addr);

		std::uint8_t msg_data_addr = cap_addr + 8;
		if (msi_cap.header.bits.addr_64_capable) {
			write_conf_reg(device, cap_addr + 8, msi_cap.msg_upper_addr);
			msg_data_addr = cap_addr + 12;
		}

		write_conf_reg(device, msg_data_addr, msi_cap.msg_data);

		if (msi_cap.header.bits.per_vector_mask_capable) {
			write_conf_reg(device, msg_data_addr + 4, msi_cap.mask_bits);
			write_conf_reg(device, msg_data_addr + 8, msi_cap.pending_bits);
		}
	}

	Error configure_msi_register(
		const Device& device,
		std::uint8_t cap_addr,
		std::uint32_t msg_addr,
		std::uint32_t msg_data,
		unsigned int num_vector_exponent) {
		auto msi_cap = read_msi_capability(device, cap_addr);
		if (msi_cap.header.bits.multi_msg_capable <= num_vector_exponent) {
			msi_cap.header.bits.multi_msg_enable = msi_cap.header.bits.multi_msg_capable;
		} else {
			msi_cap.header.bits.multi_msg_enable = num_vector_exponent;
		}

		msi_cap.header.bits.msi_enable = 1;
		msi_cap.msg_addr = msg_addr;
		msi_cap.msg_data = msg_data;

		write_msi_capability(device, cap_addr, msi_cap);
		return Error::Code::Success;
	}

	Error configure_msi(
		const Device& device,
		std::uint32_t msg_addr,
		std::uint32_t msg_data,
		unsigned int num_vector_exponent) {
		std::uint8_t cap_addr = read_conf_reg(device, 0x34) & 0xffu;
		std::uint8_t msi_cap_addr = 0;
		std::uint8_t msix_cap_addr = 0;

		while (cap_addr != 0) {
			auto header = read_capability_header(device, cap_addr);
			if (header.bits.cap_id == capability_msi) {
				msi_cap_addr = cap_addr;
			} else if (header.bits.cap_id == capability_msix) {
				msix_cap_addr = cap_addr;
			}

			cap_addr = header.bits.next_ptr;
		}

		if (msi_cap_addr != 0) {
			return configure_msi_register(device, msi_cap_addr, msg_addr, msg_data, num_vector_exponent);
		} else if (msix_cap_addr != 0) {
			return Error::Code::NotImplemented;
		}
		return Error::Code::NoPCIMSI;
	}

	Error configure_msi_fixed_destination(
		const Device& device,
		std::uint8_t apic_id,
		MSITriggerMode trigger_mode,
		MSIDeliveryMode delivery_mode,
		std::uint8_t vector,
		unsigned int num_vector_exponent) {
		std::uint32_t msg_addr = 0xfee00000u | (apic_id << 12);
		std::uint32_t msg_data = (static_cast<std::uint32_t>(delivery_mode) << 8) | vector;
		if (trigger_mode == MSITriggerMode::Level) {
			msg_data |= 0xc000;
		}

		return configure_msi(device, msg_addr, msg_data, num_vector_exponent);
	}
}
