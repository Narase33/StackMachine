#pragma once

#include <string>
#include <assert.h>

namespace base {
	class StringView {
	public:
		StringView(const char* str, size_t length)
			: str_begin(str), str_current(str), str_end(str + length) {
		}

		StringView(const std::string& str)
			: StringView(str.c_str(), str.length()) {
		}

		StringView(const char* str)
			: StringView(str, std::strlen(str)) {
		}

		void removePrefix(size_t length) {
			str_current += length;
			assert(str_current <= str_end);
		}

		void removeSuffix(size_t length) {
			str_end -= length;
			assert(str_current <= str_end);
		}

		void toPos(size_t pos) {
			str_current = str_begin + pos;
			assert(str_current <= str_end);
		}

		StringView subStr(size_t length) const {
			return StringView(str_current, length);
		}

		bool startsWith(StringView str) const {
			if (str.length() > this->length()) {
				return false;
			}
			return std::memcmp(str_current, str.c_str(), str.length()) == 0;
		}

		char operator*() const {
			return *str_current;
		}

		bool isEnd() const {
			return str_current == str_end;
		}

		size_t pos() const {
			return str_current - str_begin;
		}

		size_t length() const {
			return str_end - str_current;
		}

		const char* c_str() const {
			return str_begin;
		}

	private:
		const char* str_begin;
		const char* str_current;
		const char* str_end;
	};

	inline bool operator==(StringView left, StringView right) {
		if (left.length() != right.length()) {
			return false;
		}
		return std::memcmp(left.c_str(), right.c_str(), right.length()) == 0;
	}
}