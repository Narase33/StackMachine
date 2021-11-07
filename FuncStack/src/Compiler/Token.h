#pragma once

#include <variant>
#include "src/Base/OpCode.h"

namespace compiler {
	struct Token {
		using Type = std::variant<std::monostate, size_t, std::string>;

		Token(base::OpCode opCode, std::string value, size_t pos) : opCode(opCode), value(value), pos(pos) {}
		Token(base::OpCode opCode, size_t value, size_t pos) : opCode(opCode), value(value), pos(pos) {}
		Token(base::OpCode opCode, size_t pos) : opCode(opCode), value(std::monostate()), pos(pos) {}

		int prio() const {
			return opCodePriority(opCode);
		}

		const std::string& getString() const {
			return std::get<std::string>(value);
		}

		bool hasString() const {
			return std::holds_alternative<std::string>(value);
		}

		const size_t getNumber() const {
			return std::get<size_t>(value);
		}

		bool hasNumber() const {
			return std::holds_alternative<size_t>(value);
		}

		bool isEnd() const {
			return opCode == base::OpCode::END_PROGRAM;
		}

		base::OpCode opCode;
		Type value;
		size_t pos;
	};
}