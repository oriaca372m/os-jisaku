#include "interrupt.hpp"

void set_idt_entry(
	InterruptDescriptor& desc,
	InterruptDescriptorAttribute attr,
	std::uint64_t offset,
	std::uint16_t segment_selector) {
	desc.attr = attr;
	desc.offset_low = offset & 0xffffu;
	desc.offset_middle = (offset >> 16) & 0xffffu;
	desc.offset_high = offset >> 32;
	desc.segment_selector = segment_selector;
}

__attribute__((no_caller_saved_registers)) void notify_end_of_interrput() {
	volatile auto end_of_interrupt = reinterpret_cast<std::uint32_t*>(0xfee000b0);
	*end_of_interrupt = 0;
}
