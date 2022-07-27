#include "segment.hpp"

#include <array>

#include <asmfunc.hpp>

#include "interrupt.hpp"

namespace {
	std::array<SegmentDescriptor, 3> gdt;
}

void set_code_segment(
	SegmentDescriptor& desc,
	DescriptorType type,
	unsigned int descriptor_privilege_level,
	std::uint32_t base,
	std::uint32_t limit) {
	desc.data = 0;

	desc.bits.base_low = base & 0xffffu;
	desc.bits.base_middle = (base >> 16) & 0xffu;
	desc.bits.base_high = (base >> 24) & 0xffu;

	desc.bits.limit_low = limit & 0xffffu;
	desc.bits.limit_high = (limit >> 16) & 0xfu;

	desc.bits.type = type;
	// code & data segment
	desc.bits.system_segment = 1;
	desc.bits.descriptor_privilege_level = descriptor_privilege_level;
	desc.bits.present = 1;
	desc.bits.available = 0;
	desc.bits.long_mode = 1;
	desc.bits.default_operation_size = 0;
	desc.bits.granularity = 1;
}

void set_data_segment(
	SegmentDescriptor& desc,
	DescriptorType type,
	unsigned int descriptor_privilege_level,
	std::uint32_t base,
	std::uint32_t limit) {
	set_code_segment(desc, type, descriptor_privilege_level, base, limit);
	desc.bits.long_mode = 0;
	desc.bits.default_operation_size = 1;
}

void setup_segments() {
	gdt[0].data = 0;
	set_code_segment(gdt[1], DescriptorType::ExecuteRead, 0, 0, 0xfffff);
	set_data_segment(gdt[2], DescriptorType::ReadWrite, 0, 0, 0xfffff);
	load_gdp(sizeof(gdt) - 1, reinterpret_cast<std::uintptr_t>(&gdt[0]));
}

void initialize_segmentation() {
	setup_segments();
	const std::uint16_t kernel_cs = 1 << 3;
	const std::uint16_t kernel_ss = 2 << 3;
	set_ds_all(0);
	set_cs_ss(kernel_cs, kernel_ss);
}
