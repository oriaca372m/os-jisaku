# vim: ft=asm
	.text

# void io_out_32(std::uint16_t addr, std::uint32_t data)
.global io_out_32
io_out_32:
	mov %di, %dx
	mov %esi, %eax
	out %eax, %dx
	ret

# std::uint32_t io_in_32(std::uint16_t addr)
.global io_in_32
io_in_32:
	mov %di, %dx
	in %dx, %eax
	ret

# std::uint16_t get_cs()
.global get_cs
get_cs:
	xor %eax, %eax
	mov %cs, %ax
	ret

# void load_idt(std::uint16_t limit, std::uint64_t offset)
.global load_idt
load_idt:
	push %rbp
	mov %rsp, %rbp
	sub $10, %rsp
	# limit
	mov %di, (%rsp)
	# offset
	mov %rsi, 2(%rsp)
	lidt (%rsp)
	mov %rbp, %rsp
	pop %rbp
	ret

.global KernelMain
KernelMain:
	mov $kernel_main_stack + 1024 * 1024, %rsp
	jmp kernel_main_new_stack
