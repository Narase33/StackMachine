#pragma once

#include <variant>
#include "src/Base/OpCode.h"

namespace compiler {
	struct Token {
		using Type = std::variant<std::monostate, std::string, base::sm_int, base::sm_uint, base::sm_float, base::sm_bool>;

		Token(base::OpCode opCode, std::string value, size_t pos) : opCode(opCode), value(value), pos(pos) {}
		Token(base::OpCode opCode, base::sm_int value, size_t pos) : opCode(opCode), value(value), pos(pos) {}
		Token(base::OpCode opCode, base::sm_uint value, size_t pos) : opCode(opCode), value(value), pos(pos) {}
		Token(base::OpCode opCode, base::sm_float value, size_t pos) : opCode(opCode), value(value), pos(pos) {}
		Token(base::OpCode opCode, base::sm_bool value, size_t pos) : opCode(opCode), value(value), pos(pos) {}
		Token(base::OpCode opCode, size_t pos) : opCode(opCode), value(std::monostate{}), pos(pos) {}

		int prio() const {
			return opCodePriority(opCode);
		}

		base::OpCode opCode;
		Type value;
		size_t pos;
	};
}