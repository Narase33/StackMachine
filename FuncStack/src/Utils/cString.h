#pragma once

#include <string>
#include <ostream>

struct cString {
	template<size_t length>
	constexpr cString(char const (&str)[length])
		: str(str), length(length-1) {
	}

	const char* str;
	size_t length;
};

std::string operator+(cString left, const std::string& right) {
	return left.str + right;
}

std::string operator+(const std::string& left, cString right) {
	return left + right.str;
}

std::ostream& operator<<(std::ostream& o, cString out) {
	return o << out.str;
}