#pragma once

#include <variant>
#include "src/Base/OpCode.h"

namespace compiler {
	struct Token {
		using Type = std::variant<std::monostate, std::string, base::sm_int, base::sm_uint, base::sm_float, base::sm_bool>;

		Token(base::OpCode id, std::string value, size_t pos) : id(id), value(value), pos(pos) {}
		Token(base::OpCode id, base::sm_int value, size_t pos) : id(id), value(value), pos(pos) {}
		Token(base::OpCode id, base::sm_uint value, size_t pos) : id(id), value(value), pos(pos) {}
		Token(base::OpCode id, base::sm_float value, size_t pos) : id(id), value(value), pos(pos) {}
		Token(base::OpCode id, base::sm_bool value, size_t pos) : id(id), value(value), pos(pos) {}
		Token(base::OpCode id, size_t pos) : id(id), value(std::monostate{}), pos(pos) {}

		int prio() const {
			return opCodePriority(id);
		}

		base::OpCode id;
		Type value;
		size_t pos;
	};
}