#pragma once

#include <cstdint>

struct MemoryMap {
	std::uint64_t buffer_size;
	void* buffer;
	std::uint64_t map_size;
	std::uint64_t map_key;
	std::uint64_t descriptor_size;
	std::uint32_t descriptor_version;
};

struct MemoryDescriptor {
	std::uint32_t type;
	std::uintptr_t physical_start;
	std::uintptr_t virtual_start;
	std::uint64_t number_of_pages;
	std::uint64_t attribute;
};

enum class MemoryType : std::uint32_t {
	EfiReservedMemoryType,
	EfiLoaderCode,
	EfiLoaderData,
	EfiBootServicesCode,
	EfiBootServicesData,
	EfiRuntimeServicesCode,
	EfiRuntimeServicesData,
	EfiConventionalMemory,
	EfiUnusableMemory,
	EfiACPIReclaimMemory,
	EfiACPIMemoryNVS,
	EfiMemoryMappedIO,
	EfiMemoryMappedIOPortSpace,
	EfiPalCode,
	EfiPersistentMemory,
	EfiMaxMemoryType
};

inline bool is_available(MemoryType memory_type) {
	return memory_type == MemoryType::EfiBootServicesCode || memory_type == MemoryType::EfiBootServicesData ||
		memory_type == MemoryType::EfiConventionalMemory;
}

constexpr int uefi_page_size = 4096;
