#pragma once

#include <stdexcept>
#include <string>

using namespace std::string_literals;

namespace ex {
	class Exception : public std::runtime_error {
	public:
		using std::runtime_error::runtime_error;
	};

	void assure(bool condition, std::string&& msg) {
		if (!condition) {
			throw Exception(std::forward<std::string>(msg));
		}
	}

	void assure(bool condition, const char* msg) {
		if (!condition) {
			throw Exception(msg);
		}
	}
}