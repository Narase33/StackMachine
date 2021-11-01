#pragma once

#include <string>
#include <assert.h>

class StringWindow {
public:
	StringWindow(const char* str, size_t length)
		: str_begin(str), window_begin(str), window_end(str + length), str_end(str + length) {
	}

	StringWindow(const std::string& str)
		: StringWindow(str.c_str(), str.length()) {
	}

	StringWindow(const char* str)
		: StringWindow(str, std::strlen(str)) {
	}

	void removePrefix(size_t length) {
		window_begin += length;
		assert(window_begin <= window_end);
	}

	void addPrefix(size_t length) {
		window_begin -= length;
		assert(window_begin >= str_begin);
	}

	void removeSuffix(size_t length) {
		window_end -= length;
		assert(window_end >= window_begin);
	}

	void addSuffix(size_t length) {
		window_end += length;
		assert(window_end <= str_end);
	}

	StringWindow subStr(size_t length) const {
		StringWindow copy = *this;
		copy.window_end = copy.window_begin + length;
		return copy;
	}

	template<size_t length>
	constexpr size_t startsWith(char const (&str)[length]) const {
		if (length > this->length()) {
			return false;
		}
		return std::memcmp(window_begin, str, length - 1) == 0;
	}

	char operator*() const {
		return *window_begin;
	}

	char operator[](size_t index) const {
		return window_begin[index];
	}

	bool isEnd() const {
		return window_begin == window_end;
	}

	size_t pos() const {
		return window_begin - str_begin;
	}

	size_t length() const {
		return window_end - window_begin;
	}

	const char* c_str() const {
		return window_begin;
	}

	std::string str() const {
		return std::string(window_begin, window_end);
	}
private:
	const char* str_begin;
	const char* window_begin;
	const char* window_end;
	const char* str_end;
};

template<size_t length>
inline bool operator==(StringWindow left, char const (&right)[length]) {
	if (left.length() != (length - 1)) {
		return false;
	}
	return std::memcmp(left.c_str(), right, length - 1) == 0;
}