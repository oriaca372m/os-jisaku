#pragma once

#include <array>
#include <cstdint>

enum class DescriptorType {
	Upper8Bytes = 0,
	LDT = 2,
	TSSAvailable = 9,
	TSSBusy = 11,
	CallGate = 12,
	InterruptGate = 14,
	TrapGate = 15,
};

union InterruptDescriptorAttribute {
	std::uint16_t data;
	struct {
		std::uint8_t interrupt_stack_table : 3;
		std::uint8_t : 5;
		DescriptorType type : 4;
		std::uint8_t : 1;
		std::uint8_t descriptor_privilege_level : 2;
		std::uint8_t present : 1;
	} __attribute__((packed)) bits;
} __attribute__((packed));

struct InterruptDescriptor {
	std::uint16_t offset_low;
	std::uint16_t segment_selector;
	InterruptDescriptorAttribute attr;
	std::uint16_t offset_middle;
	std::uint32_t offset_high;
	std::uint32_t reserved;
};

inline std::array<InterruptDescriptor, 256> idt{};

constexpr InterruptDescriptorAttribute make_idt_attr(
	DescriptorType type,
	std::uint8_t descriptor_privilege_level,
	bool present = true,
	std::uint8_t interrupt_stack_table = 0) {
	InterruptDescriptorAttribute attr{};
	attr.bits.interrupt_stack_table = interrupt_stack_table;
	attr.bits.type = type;
	attr.bits.descriptor_privilege_level = descriptor_privilege_level;
	attr.bits.present = present;
	return attr;
}

void set_idt_entry(
	InterruptDescriptor& desc,
	InterruptDescriptorAttribute attr,
	std::uint64_t offset,
	std::uint16_t segment_selector);

namespace InterruptVector {
	inline constexpr std::size_t xhci = 0x40;
};

struct InterruptFrame {
	std::uint64_t rip;
	std::uint64_t cs;
	std::uint64_t rflags;
	std::uint64_t rsp;
	std::uint64_t ss;
};

void notify_end_of_interrput();
