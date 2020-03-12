#pragma once

#include <functional>
#include <array>
#include <unordered_map>
#include <optional>

#include "Type.h"

namespace base {
	enum class Operator {
		JUMP, JUMP_IF, JUMP_LABEL, JUMP_LABEL_IF, LABEL, ERR,
		LOAD, STORE, POP,
		IF, ELSE,
		BRACE_GROUP, BRACE_OPEN, BRACE_CLOSE, BRACKET_GROUP, BRACKET_OPEN, BRACKET_CLOSE,
		EQ, UNEQ,
		BIGGER, LESS,
		ADD, SUB,
		MULT, DIV,
		INCR, DECR,
	};

	struct OperatorAttribute {
		Operator op;
		const char* name;
		const char* symbol;
		int impact;
		int priority;

		constexpr OperatorAttribute(Operator op, const char* name, const char* symbol, int impact, int priority)
			: op(op), name(name), symbol(symbol), impact(impact), priority(priority) {
		}
	};

	constexpr auto attributes = {
		OperatorAttribute(Operator::IF, "IF", "if", 0, 0), // Lexer, Parser
		OperatorAttribute(Operator::ELSE, "ELSE", "else", 0, 0), // Lexer, Parser
		OperatorAttribute(Operator::BRACE_OPEN, "BRACE_OPEN", "(", 0, 0), // Lexer, Parser
		OperatorAttribute(Operator::BRACE_CLOSE, "BRACE_CLOSE", ")", 0, 0), // Lexer, Parser
		OperatorAttribute(Operator::BRACKET_OPEN, "BRACKET_OPEN", "{", 0, 0), // Lexer, Parser
		OperatorAttribute(Operator::BRACKET_CLOSE, "BRACKET_CLOSE", "}", 0, 0), // Lexer, Parser

		OperatorAttribute(Operator::LOAD, "LOAD", "", 1,  0), // Lexer, Parser, Exec
		OperatorAttribute(Operator::STORE, "STORE", "=", -1, 0), // Lexer, Parser, Exec

		OperatorAttribute(Operator::EQ, "EQ", "==", -1, 1), // Lexer, Parser, Exec
		OperatorAttribute(Operator::UNEQ, "UNEQ", "!=", -1, 1), // Lexer, Parser, Exec

		OperatorAttribute(Operator::BIGGER, "BIGGER", ">", -1, 2), // Lexer, Parser, Exec
		OperatorAttribute(Operator::LESS, "LESS", "<", -1, 2), // Lexer, Parser, Exec

		OperatorAttribute(Operator::ADD, "ADD", "+", -1, 3), // Lexer, Parser, Exec
		OperatorAttribute(Operator::SUB, "SUB", "-", -1, 3), // Lexer, Parser, Exec

		OperatorAttribute(Operator::MULT, "MULT", "*", -1, 4), // Lexer, Parser, Exec
		OperatorAttribute(Operator::DIV, "DIV", "/", -1, 4), // Lexer, Parser, Exec

		OperatorAttribute(Operator::INCR, "INCR", "++", 0, 5), // Lexer, Parser, Exec
		OperatorAttribute(Operator::DECR, "DECR", "--", 0, 5), // Lexer, Parser, Exec

		OperatorAttribute(Operator::ERR, "ERROR", "", 0, 0), // Parser
		OperatorAttribute(Operator::BRACE_GROUP, "BRACE_GROUP", "", 0, 0), // Parser
		OperatorAttribute(Operator::BRACKET_GROUP, "BRACKET_GROUP", "", 0, 0), // Parser

		OperatorAttribute(Operator::JUMP_LABEL, "JUMP_LABEL", "", 0, 0), // Parser
		OperatorAttribute(Operator::JUMP_LABEL_IF, "JUMP_LABEL_IF", "", 0, 0), // Parser
		OperatorAttribute(Operator::LABEL, "LABEL", "", 0, 0), // Parser

		OperatorAttribute(Operator::JUMP, "JUMP", "", 0, 0), // Parser, Exec
		OperatorAttribute(Operator::JUMP_IF, "JUMP_IF", "", 0, 0), // Parser, Exec
		
		OperatorAttribute(Operator::POP, "POP", "", -1, 0), // Parser, Exec

	};

	constexpr OperatorAttribute getAttribute(Operator op) {
		for (const auto& i : attributes) {
			if (i.op == op) {
				return i;
			}
		}
		throw std::exception("Operator not found");
	}

	constexpr const char* getName(Operator op) {
		return getAttribute(op).name;
	}

	constexpr int getPriority(Operator op) {
		return getAttribute(op).priority;
	}

	constexpr OperatorAttribute fromSymbol(const char* symbol) {
		for (const auto& i : attributes) {
			if (std::strcmp(i.symbol, symbol) == 0) {
				return i;
			}
		}
		throw std::exception("Operator-Symbol not found");
	}

	constexpr bool isSymbol(const char* symbol) {
		for (const auto& i : attributes) {
			if (std::strcmp(i.symbol, symbol) == 0) {
				return true;
			}
		}
		return false;
	}

	constexpr bool producesBool(Operator op) {
		switch (op) {
			case Operator::BIGGER:
			case Operator::LESS:
			case Operator::EQ:
			case Operator::UNEQ: return true;
		}

		return false;
	}

	constexpr bool bigger(Operator a, Operator b) {
		return getPriority(a) > getPriority(b);
	}

	constexpr int maxSize() {
		size_t i = 0;
		for (const auto& op : attributes) {
			i = std::max(i, std::strlen(op.symbol));
		}
		return i;
	}

	constexpr Operator toGroupBrace(Operator brace) {
		switch (brace) {
			case Operator::BRACE_OPEN:
			case Operator::BRACE_CLOSE: return Operator::BRACE_GROUP;
			case Operator::BRACKET_OPEN:
			case Operator::BRACKET_CLOSE: return Operator::BRACKET_GROUP;
			default:
				throw std::exception("Unknown group operator");
		}
	}

	constexpr std::pair<Operator, Operator> FromGroupBrace(Operator brace) {
		switch (brace) {
			case Operator::BRACE_GROUP: return std::make_pair(Operator::BRACE_OPEN, Operator::BRACE_CLOSE);
			case Operator::BRACKET_GROUP: return std::make_pair(Operator::BRACKET_OPEN, Operator::BRACKET_CLOSE);
			default:
				throw std::exception("Unknown group operator");
		}
	}
}