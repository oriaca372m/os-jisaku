cmake_minimum_required(VERSION 3.20.3)
project(Kernel)

set(CMAKE_C_COMPILER "/usr/bin/clang")
set(CMAKE_CXX_COMPILER "/usr/bin/clang++")
set(CMAKE_C_LINK_EXECUTABLE "/usr/bin/ld.lld <LINK_FLAGS> <OBJECTS> <LINK_LIBRARIES> -o <TARGET>")
set(CMAKE_CXX_LINK_EXECUTABLE "/usr/bin/ld.lld <LINK_FLAGS> <OBJECTS> <LINK_LIBRARIES> -o <TARGET>")

add_executable(kernel.elf main.cpp)

target_include_directories(kernel.elf PUBLIC "/usr/include/c++/v1")
target_compile_definitions(kernel.elf PUBLIC __ELF__ _LDBL_EQ_DBL _GNU_SOURCE _POSIX_TIMERS _LIBCPP_HAS_NO_THREADS)

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
