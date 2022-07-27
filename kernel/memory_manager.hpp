#pragma once

#include <array>
#include <cstddef>
#include <limits>

#include "error.hpp"
#include "memory_map.hpp"

namespace {
	constexpr unsigned long long operator""_kib(unsigned long long kib) {
		return kib * 1024;
	}

	constexpr unsigned long long operator""_mib(unsigned long long mib) {
		return mib * 1024_kib;
	}

	constexpr unsigned long long operator""_gib(unsigned long long gib) {
		return gib * 1024_mib;
	}
}

constexpr auto bytes_per_frame = 4_kib;

class FrameID final {
public:
	explicit FrameID(std::size_t id) : id_{id} {}

	std::size_t id() const {
		return id_;
	}

	void* frame() const {
		return reinterpret_cast<void*>(id_ * bytes_per_frame);
	}

private:
	std::size_t id_;
};

inline const FrameID null_frame(std::numeric_limits<std::size_t>::max());

class BitmapMemoryManager final {
public:
	static constexpr auto max_physical_memory_bytes = 128_gib;
	static constexpr auto frame_count = max_physical_memory_bytes / bytes_per_frame;

	using MapLineType = unsigned long;
	static constexpr std::size_t bits_per_map_line = 8 * sizeof(MapLineType);

	BitmapMemoryManager();

	WithError<FrameID> allocate(std::size_t num_frames);
	Error free(FrameID start_frame, std::size_t num_frames);

	void mark_allocated(FrameID start_frame, std::size_t num_frames);

	void set_memory_range(FrameID range_begin, FrameID range_end);

private:
	std::array<MapLineType, frame_count / bits_per_map_line> alloc_map_;

	FrameID range_begin_;
	FrameID range_end_;

	bool get_bit(FrameID frame) const;
	void set_bit(FrameID frame, bool allocated);
};

void initialize_memory_manager(const MemoryMap& memory_map, BitmapMemoryManager& memory_manager);
