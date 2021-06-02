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
