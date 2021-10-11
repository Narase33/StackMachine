#pragma once

#include <stdexcept>
#include <string>

using namespace std::string_literals;

namespace ex {
	class Exception : public std::runtime_error {
	public:
		using std::runtime_error::runtime_error;
	};

	class ParserException : public Exception {
		size_t pos;
	public:
		ParserException(const std::string& message, size_t pos) :
			Exception(message), pos(pos) {
		}

		size_t getPos() const {
			return pos;
		}
	};

	void assume(bool condition, const std::string& message) {
		if (!condition) {
			throw ex::Exception(message);
		}
	}
}