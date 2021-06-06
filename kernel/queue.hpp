#pragma once

#include <array>
#include <cstddef>

#include "error.hpp"

template <typename T>
class ArrayQueue {
public:
	template <std::size_t N>
	ArrayQueue(std::array<T, N>& buf) : ArrayQueue(buf.data(), N) {}

	ArrayQueue(T* buf, std::size_t size) : data_{buf}, capacity_{size} {}

	Error push(const T& value) {
		if (count_ == capacity_) {
			return Error::Code::Full;
		}

		data_[write_pos_] = value;

		++count_;
		++write_pos_;
		if (write_pos_ == capacity_) {
			write_pos_ = 0;
		}

		return Error::Code::Success;
	}

	Error pop() {
		if (count_ == 0) {
			return Error::Code::Empty;
		}

		--count_;
		++read_pos_;
		if (read_pos_ == capacity_) {
			read_pos_ = 0;
		}

		return Error::Code::Success;
	}

	const T& front() const {
		return data_[read_pos_];
	}

	std::size_t count() const {
		return count_;
	}
	std::size_t capacity() const {
		return capacity_;
	}

private:
	T* data_;
	const std::size_t capacity_;

	std::size_t read_pos_ = 0;
	std::size_t write_pos_ = 0;
	std::size_t count_ = 0;
};
