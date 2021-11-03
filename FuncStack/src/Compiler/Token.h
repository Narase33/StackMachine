#pragma once

#include <variant>
#include "src/Base/OpCode.h"

namespace compiler {
	struct Token {
		using Type = std::variant<std::monostate, std::string, base::sm_int, base::sm_uint, base::sm_float, base::sm_bool>; // TODO add TypeIndex

		Token(base::OpCode opCode, std::string value, size_t pos) : opCode(opCode), value(value), pos(pos) {}
		Token(base::OpCode opCode, base::sm_int value, size_t pos) : opCode(opCode), value(value), pos(pos) {}
		Token(base::OpCode opCode, base::sm_uint value, size_t pos) : opCode(opCode), value(value), pos(pos) {}
		Token(base::OpCode opCode, base::sm_float value, size_t pos) : opCode(opCode), value(value), pos(pos) {}
		Token(base::OpCode opCode, base::sm_bool value, size_t pos) : opCode(opCode), value(value), pos(pos) {}
		Token(base::OpCode opCode, size_t pos) : opCode(opCode), value(std::monostate{}), pos(pos) {}

		int prio() const {
			return opCodePriority(opCode);
		}

		template<typename T>
		const T& get() const {
			return std::get<T>(value);
		}

		bool isEnd() const {
			return opCode == base::OpCode::END_PROGRAM;
		}

		base::OpCode opCode;
		Type value;
		size_t pos;
	};

	base::BasicType literalToValue(const Token& token) {
		if (std::holds_alternative<base::sm_int>(token.value)) {
			return base::BasicType(std::get<base::sm_int>(token.value));
		} else if (std::holds_alternative<base::sm_uint>(token.value)) {
			return base::BasicType(std::get<base::sm_uint>(token.value));
		} else if (std::holds_alternative<base::sm_float>(token.value)) {
			return base::BasicType(std::get<base::sm_float>(token.value));
		} else if (std::holds_alternative<base::sm_bool>(token.value)) {
			return base::BasicType(std::get<base::sm_bool>(token.value));
		}
		throw ex::ParserException("Unknown value", token.pos);
	}
}