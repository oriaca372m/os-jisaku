#pragma once

#include <array>

class Error {
public:
	enum class Code {
		Success,
		Full,
		Empty,
		IndexOutOfRange,
		NotImplemented,
		NoPCIMSI,
		NoEnoughMemory,
		LastOfCode,
	};

	Error(Code code) : code_{code} {}

	operator bool() const {
		return this->code_ != Code::Success;
	}

	const char* name() const {
		auto num_code = static_cast<int>(code_);
		if (num_code < code_names_.size()) {
			return code_names_[num_code];
		}

		return u8"Unknown";
	}

private:
	static constexpr std::array code_names_ = {
		u8"Success",
		u8"Full",
		u8"Empty",
		u8"IndexOutOfRange",
		u8"NotImplemented",
		u8"NoPCIMSI",
		u8"NoEnoughMemory",
	};

	Code code_;
};

template <typename T>
struct WithError {
	T value;
	Error error;
};
