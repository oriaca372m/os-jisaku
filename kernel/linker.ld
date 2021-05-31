SECTIONS
{
	. = 0x100000 + SIZEOF_HEADERS;
	.text : { *(.text) }
}
