cmake_minimum_required(VERSION 3.20.3)
project(Kernel)

set(CMAKE_C_COMPILER "/usr/bin/clang")
set(CMAKE_CXX_COMPILER "/usr/bin/clang++")
set(CMAKE_C_LINK_EXECUTABLE "/usr/bin/ld.lld <LINK_FLAGS> <OBJECTS> <LINK_LIBRARIES> -o <TARGET>")
set(CMAKE_CXX_LINK_EXECUTABLE "/usr/bin/ld.lld <LINK_FLAGS> <OBJECTS> <LINK_LIBRARIES> -o <TARGET>")

add_executable(kernel.elf
	main.cpp
	graphics.cpp
	font.cpp
	newlib_support.cpp
)

target_include_directories(kernel.elf PUBLIC
	"${CMAKE_CURRENT_SOURCE_DIR}/../stdlib/build/include/c++/v1"
	"${CMAKE_CURRENT_SOURCE_DIR}/../stdlib/build/include"
)
target_compile_definitions(kernel.elf PUBLIC __ELF__ _LDBL_EQ_DBL _GNU_SOURCE _POSIX_TIMERS)

set_property(TARGET kernel.elf PROPERTY CXX_STANDARD 17)
target_compile_options(kernel.elf
	PUBLIC -O2
	PUBLIC -Wall
	PUBLIC -g
	PUBLIC --target=x86_64-elf
	PUBLIC -ffreestanding
	PUBLIC -mno-red-zone
	PUBLIC -fno-exceptions
	PUBLIC -fno-rtti
	PUBLIC -nostdlibinc
)

set(LINKER_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/linker.ld")
set_target_properties(kernel.elf PROPERTIES LINK_DEPENDS ${LINKER_SCRIPT})
target_link_options(kernel.elf
	PUBLIC --entry KernelMain
	PUBLIC -z norelro
	PUBLIC --static
	PUBLIC -T ${LINKER_SCRIPT}
)

add_custom_command(
	OUTPUT hankaku.bin
	COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/../tools/makefont.py" -o hankaku.bin "${CMAKE_CURRENT_SOURCE_DIR}/../assets/hankaku.txt"
	DEPENDS ../assets/hankaku.txt
	VERBATIM
)

add_custom_command(
	OUTPUT hankaku.o
	COMMAND objcopy -I binary -O elf64-x86-64 -B i386:x86-64 hankaku.bin hankaku.o
	DEPENDS hankaku.bin
	VERBATIM
)

add_library(shinonome_font_bin OBJECT IMPORTED)
set_property(TARGET shinonome_font_bin PROPERTY IMPORTED_OBJECTS "${CMAKE_CURRENT_BINARY_DIR}/hankaku.o")

target_link_libraries(kernel.elf PUBLIC shinonome_font_bin)
target_link_libraries(kernel.elf PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../stdlib/build/lib/libc.a")
