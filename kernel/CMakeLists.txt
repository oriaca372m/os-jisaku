cmake_minimum_required(VERSION 3.20.3)

set(CMAKE_C_COMPILER "/usr/bin/clang")
set(CMAKE_CXX_COMPILER "/usr/bin/clang++")
set(CMAKE_C_LINK_EXECUTABLE "/usr/bin/ld.lld <LINK_FLAGS> <OBJECTS> <LINK_LIBRARIES> -o <TARGET>")
set(CMAKE_CXX_LINK_EXECUTABLE "/usr/bin/ld.lld <LINK_FLAGS> <OBJECTS> <LINK_LIBRARIES> -o <TARGET>")

project(Kernel)

add_executable(kernel.elf
	main.cpp
	support_functions.cpp
	pci.cpp
	utils.cpp
	kernel_interface_impl.cpp
	logger.cpp
	interrupt.cpp
	segment.cpp
	paging.cpp
	memory_manager.cpp
	sbrk.cpp
	timer.cpp
	window.cpp
	graphics/graphics.cpp
	graphics/font.cpp
	graphics/console.cpp
	graphics/mouse.cpp
	graphics/layer.cpp
	graphics/layer_manager.cpp
	graphics/painter.cpp
	graphics/frame_buffer.cpp
	graphics/device_pixel_writer.cpp
)

target_include_directories(kernel.elf PUBLIC
	"${CMAKE_CURRENT_SOURCE_DIR}/../stdlib/build/include/c++/v1"
	"${CMAKE_CURRENT_SOURCE_DIR}/../stdlib/build/include"
)
target_include_directories(kernel.elf PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
target_compile_definitions(kernel.elf PUBLIC __ELF__ _LDBL_EQ_DBL _GNU_SOURCE _POSIX_TIMERS)

set_property(TARGET kernel.elf PROPERTY CXX_STANDARD 17)
set_property(TARGET kernel.elf PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
target_compile_options(kernel.elf PUBLIC
	-O2
	-Wall
	-g
	--target=x86_64-elf
	-ffreestanding
	-mno-red-zone
	-fno-exceptions
	-fno-rtti
	-nostdlibinc
)

set(LINKER_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/linker.ld")
set_target_properties(kernel.elf PROPERTIES LINK_DEPENDS ${LINKER_SCRIPT})
target_link_options(kernel.elf PUBLIC
	--entry=kernel_main
	"SHELL:-z norelro"
	"SHELL:-z stack-size=0x100000"
	--static
	--script=${LINKER_SCRIPT}
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
target_link_libraries(kernel.elf PUBLIC
	"${CMAKE_CURRENT_SOURCE_DIR}/../stdlib/build/lib/libc.a"
	"${CMAKE_CURRENT_SOURCE_DIR}/../stdlib/build/lib/libc++.a"
	"${CMAKE_CURRENT_SOURCE_DIR}/../stdlib/build/lib/libc++abi.a"
)

add_subdirectory(asmfunc)
target_link_libraries(kernel.elf PUBLIC asmfunc)

add_subdirectory(kernel_interface)
target_link_libraries(kernel.elf PUBLIC kernel_interface)

add_subdirectory(usb)
target_link_libraries(kernel.elf PUBLIC usb)
