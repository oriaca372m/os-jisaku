#pragma once

#include <array>

class Error {
public:
	enum class Code {
		Success,
		Full,
		Empty,
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

		return "Unknown";
	}

private:
	static constexpr std::array<const char*, 3> code_names_ = {
		u8"Success",
		u8"Full",
		u8"Empty",
	};

	Code code_;
};
