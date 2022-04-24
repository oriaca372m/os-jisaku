cmake_minimum_required(VERSION 3.20.3)

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
set_property(TARGET kernel.elf PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
target_compile_options(usb PUBLIC
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

target_link_options(usb PUBLIC "SHELL:-z norelro")

target_link_libraries(usb PRIVATE kernel_interface)
