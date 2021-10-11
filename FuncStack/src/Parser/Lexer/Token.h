#pragma once

#include <variant>
#include "src/Base/OpCode.h"

struct Token {
	using Type = std::variant<std::monostate, std::string, sm_int, sm_uint, sm_float, sm_bool>;

	Token(OpCode id, std::string value, size_t pos) : id(id), value(value), pos(pos) {}
	Token(OpCode id, sm_int value, size_t pos) : id(id), value(value), pos(pos) {}
	Token(OpCode id, sm_uint value, size_t pos) : id(id), value(value), pos(pos) {}
	Token(OpCode id, sm_float value, size_t pos) : id(id), value(value), pos(pos) {}
	Token(OpCode id, sm_bool value, size_t pos) : id(id), value(value), pos(pos) {}
	Token(OpCode id, size_t pos) : id(id), value(std::monostate{}), pos(pos) {}

	int prio() const {
		return opCodePriority(id);
	}

	OpCode id;
	Type value;
	size_t pos;
};