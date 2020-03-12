#pragma once

#include <exception>
#include <string>

using namespace std::string_literals;

namespace ex {
	class Exception : public std::exception {
		const std::string msg;
	public:
		Exception(std::string&& msg) : msg(std::move(msg)) {}

		const char* what() const override {
			return msg.c_str();
		}
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