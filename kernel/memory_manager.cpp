#include "memory_manager.hpp"

void BitmapMemoryManager::mark_allocated(FrameID start_frame, std::size_t num_frames) {
	for (std::size_t i = 0; i < num_frames; ++i) {
		set_bit(FrameID(start_frame.id() + i), true);
	}
}

void BitmapMemoryManager::set_memory_range(FrameID range_begin, FrameID range_end) {
	range_begin_ = range_begin;
	range_end_ = range_end;
}

bool BitmapMemoryManager::get_bit(FrameID frame) const {
	const auto line_index = frame.id() / bits_per_map_line;
	const auto bit_index = frame.id() % bits_per_map_line;
	return (alloc_map_[line_index] & (static_cast<MapLineType>(1) << bit_index)) != 0;
}

void BitmapMemoryManager::set_bit(FrameID frame, bool allocated) {
	const auto line_index = frame.id() / bits_per_map_line;
	const auto bit_index = frame.id() % bits_per_map_line;

	if (allocated) {
		alloc_map_[line_index] |= static_cast<MapLineType>(1) << bit_index;
	} else {
		alloc_map_[line_index] &= ~(static_cast<MapLineType>(1) << bit_index);
	}
}

WithError<FrameID> BitmapMemoryManager::allocate(std::size_t num_frames) {
	std::size_t start_frame_id = range_begin_.id();
	while (true) {
		std::size_t i = 0;

		for (; i < num_frames; ++i) {
			if (range_end_.id() <= start_frame_id + i) {
				return {null_frame, Error::Code::NoEnoughMemory};
			}

			if (get_bit(FrameID(start_frame_id + i))) {
				break;
			}
		}

		if (i == num_frames) {
			const FrameID start_frame(start_frame_id);
			mark_allocated(start_frame, num_frames);
			return {start_frame, Error::Code::Success};
		}

		start_frame_id += i + 1;
	}
}

Error BitmapMemoryManager::free(FrameID start_frame, std::size_t num_frames) {
	for (std::size_t i = 0; i < num_frames; ++i) {
		set_bit(FrameID(start_frame.id() + i), false);
	}

	return Error::Code::Success;
}
