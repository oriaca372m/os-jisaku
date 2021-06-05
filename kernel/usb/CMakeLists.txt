cmake_minimum_required(VERSION 3.20.3)

set(CMAKE_C_COMPILER "/usr/bin/clang")
set(CMAKE_CXX_COMPILER "/usr/bin/clang++")
set(CMAKE_C_LINK_EXECUTABLE "/usr/bin/ld.lld <LINK_FLAGS> <OBJECTS> <LINK_LIBRARIES> -o <TARGET>")
set(CMAKE_CXX_LINK_EXECUTABLE "/usr/bin/ld.lld <LINK_FLAGS> <OBJECTS> <LINK_LIBRARIES> -o <TARGET>")

add_library(usb STATIC
	classdriver/base.cpp
	classdriver/hid.cpp
	classdriver/keyboard.cpp
	classdriver/mouse.cpp
	xhci/device.cpp
	xhci/devmgr.cpp
	xhci/port.cpp
	xhci/registers.cpp
	xhci/ring.cpp
	xhci/trb.cpp
	xhci/xhci.cpp
	device.cpp
	memory.cpp
	logger.cpp
)

target_include_directories(usb PUBLIC
	"${CMAKE_CURRENT_SOURCE_DIR}/../../stdlib/build/include/c++/v1"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../stdlib/build/include"
	"${CMAKE_CURRENT_SOURCE_DIR}/interface"
)
target_include_directories(usb PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
target_compile_definitions(usb PUBLIC __ELF__ _LDBL_EQ_DBL _GNU_SOURCE _POSIX_TIMERS)

set_property(TARGET usb PROPERTY CXX_STANDARD 17)
target_compile_options(usb
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

set_target_properties(usb PROPERTIES LINK_DEPENDS ${LINKER_SCRIPT})
target_link_options(usb
	PUBLIC -z norelro
)

target_link_libraries(usb PRIVATE kernel_interface)
