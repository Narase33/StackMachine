#pragma once

#include "src/Exception.h"
#include "StringView.h"

namespace base {
	enum class OpCode : uint8_t {
		// Global
		ERR, NOOP,

		// Lexer
		END_STATEMENT, COMMA,
		IF, ELSE, WHILE,
		CONTINUE, BREAK,
		BRACKET_ROUND_OPEN, BRACKET_ROUND_CLOSE,
		BRACKET_CURLY_OPEN, BRACKET_CURLY_CLOSE,
		BRACKET_SQUARE_OPEN, BRACKET_SQUARE_CLOSE,
		TYPE,
		NAME, FUNC,

		// Lexer + Interpreter
		LOAD_LITERAL,
		EQ, UNEQ,
		BIGGER, LESS,
		ADD, SUB,
		MULT, DIV,
		INCR, DECR,
		ASSIGN,

		// Interpreter
		BEGIN_SCOPE, END_SCOPE, CALL_FUNCTION, END_FUNCTION,
		END_PROGRAM,
		LOAD_LOCAL, LOAD_GLOBAL, STORE_LOCAL, STORE_GLOBAL,
		JUMP, JUMP_IF_NOT,
		CREATE_VARIABLE, POP,
		END_ENUM_OPCODE
	};

	inline OpCode opCodeFromKeyword(StringView keyword) {
		if (keyword == "if") return OpCode::IF;
		if (keyword == "else") return OpCode::ELSE;
		if (keyword == "while") return OpCode::WHILE;
		if (keyword == "continue") return OpCode::CONTINUE;
		if (keyword == "break") return OpCode::BREAK;
		if (keyword == "func") return OpCode::FUNC;
		return OpCode::ERR;
	}

	inline OpCode opCodeFromSymbol(StringView symbol) {
		if (symbol.length() == 1) {
			switch (*symbol) {
				case '(': return OpCode::BRACKET_ROUND_OPEN;
				case ')': return OpCode::BRACKET_ROUND_CLOSE;
				case '{': return OpCode::BRACKET_CURLY_OPEN;
				case '}': return  OpCode::BRACKET_CURLY_CLOSE;
				case '[': return OpCode::BRACKET_SQUARE_OPEN;
				case ']': return  OpCode::BRACKET_SQUARE_CLOSE;
				case '>': return OpCode::BIGGER;
				case '<': return OpCode::LESS;
				case '+': return OpCode::ADD;
				case '-': return OpCode::SUB;
				case '*': return OpCode::MULT;
				case '/': return OpCode::DIV;
				case '=': return OpCode::ASSIGN;
				case ';': return OpCode::END_STATEMENT;
			}
		}

		if (symbol == "++") return OpCode::INCR;
		if (symbol == "--") return OpCode::DECR;
		if (symbol == "==") return OpCode::EQ;
		if (symbol == "!=") return OpCode::UNEQ;
		return OpCode::ERR;
	}

	inline int opCodePriority(OpCode opCode) {
		switch (opCode) {
			case OpCode::LOAD_LITERAL: return 1;
			case OpCode::NAME: return 0;
			case OpCode::BRACKET_ROUND_OPEN: return 0;
			case OpCode::BRACKET_CURLY_OPEN: return 0;
			case OpCode::BRACKET_SQUARE_OPEN: return 0;
			case OpCode::EQ: return 1;
			case OpCode::UNEQ: return 1;
			case OpCode::BIGGER: return 2;
			case OpCode::LESS: return 2;
			case OpCode::ADD: return 3;
			case OpCode::SUB: return 3;
			case OpCode::MULT: return 4;
			case OpCode::DIV: return 4;
			case OpCode::INCR: return 5;
			case OpCode::DECR: return 5;
		}
		return -1;
	}

	inline int opCodeImpact(OpCode opCode) {
		switch (opCode) {
			case base::OpCode::LOAD_LOCAL:
			case base::OpCode::NAME:
			case base::OpCode::LOAD_LITERAL: return 1;
			case base::OpCode::JUMP:
			case base::OpCode::JUMP_IF_NOT:
			case base::OpCode::BEGIN_SCOPE:
			case base::OpCode::END_SCOPE:
			case base::OpCode::INCR:
			case base::OpCode::DECR:
			case base::OpCode::CREATE_VARIABLE: return 0;
			case base::OpCode::EQ:
			case base::OpCode::UNEQ:
			case base::OpCode::LESS:
			case base::OpCode::BIGGER:
			case base::OpCode::ADD:
			case base::OpCode::SUB:
			case base::OpCode::MULT:
			case base::OpCode::DIV:
			case base::OpCode::STORE_LOCAL: return -1;
		}
		throw ex::Exception("Unknown opCode " + std::to_string(static_cast<size_t>(opCode)));
	}

	inline std::pair<OpCode, OpCode> getBracketGroup(OpCode openBracket) {
		switch (openBracket) {
			case OpCode::BRACKET_ROUND_OPEN:
			case OpCode::BRACKET_ROUND_CLOSE: return { OpCode::BRACKET_ROUND_OPEN, OpCode::BRACKET_ROUND_CLOSE };
			case OpCode::BRACKET_CURLY_OPEN:
			case OpCode::BRACKET_CURLY_CLOSE: return { OpCode::BRACKET_CURLY_OPEN, OpCode::BRACKET_CURLY_CLOSE };
			case OpCode::BRACKET_SQUARE_OPEN:
			case OpCode::BRACKET_SQUARE_CLOSE: return { OpCode::BRACKET_SQUARE_OPEN, OpCode::BRACKET_SQUARE_CLOSE };
			default: return { OpCode::ERR, OpCode::ERR };
		}
	}

	inline bool isOpeningBracket(OpCode op) {
		switch (op) {
			case OpCode::BRACKET_ROUND_OPEN:
			case OpCode::BRACKET_CURLY_OPEN:
			case OpCode::BRACKET_SQUARE_OPEN:
				return true;
			default: return false;
		}
	}

	inline bool isClosingBracket(OpCode op) {
		switch (op) {
			case OpCode::BRACKET_ROUND_CLOSE:
			case OpCode::BRACKET_CURLY_CLOSE:
			case OpCode::BRACKET_SQUARE_CLOSE:
				return true;
			default: return false;
		}
	}

	inline bool isBracket(OpCode op) {
		return isOpeningBracket(op) or isClosingBracket(op);
	}

#define SM_REGISTER_NAME(x) case x: return #x
	inline std::string opCodeName(OpCode opCode) {
		switch (opCode) {
			// Global
			SM_REGISTER_NAME(OpCode::NOOP);
			SM_REGISTER_NAME(OpCode::ERR);

			// Lexer
			SM_REGISTER_NAME(OpCode::TYPE);
			SM_REGISTER_NAME(OpCode::END_STATEMENT);
			SM_REGISTER_NAME(OpCode::COMMA);
			SM_REGISTER_NAME(OpCode::IF);
			SM_REGISTER_NAME(OpCode::ELSE);
			SM_REGISTER_NAME(OpCode::WHILE);
			SM_REGISTER_NAME(OpCode::CONTINUE);
			SM_REGISTER_NAME(OpCode::BREAK);
			SM_REGISTER_NAME(OpCode::BRACKET_ROUND_OPEN);
			SM_REGISTER_NAME(OpCode::BRACKET_ROUND_CLOSE);
			SM_REGISTER_NAME(OpCode::BRACKET_SQUARE_OPEN);
			SM_REGISTER_NAME(OpCode::BRACKET_SQUARE_CLOSE);
			SM_REGISTER_NAME(OpCode::BRACKET_CURLY_OPEN);
			SM_REGISTER_NAME(OpCode::BRACKET_CURLY_CLOSE);
			SM_REGISTER_NAME(OpCode::NAME);
			SM_REGISTER_NAME(OpCode::FUNC);

			// Lexer + Interpreter
			SM_REGISTER_NAME(OpCode::LOAD_LITERAL);
			SM_REGISTER_NAME(OpCode::BEGIN_SCOPE);
			SM_REGISTER_NAME(OpCode::END_SCOPE);
			SM_REGISTER_NAME(OpCode::CALL_FUNCTION);
			SM_REGISTER_NAME(OpCode::END_FUNCTION);
			SM_REGISTER_NAME(OpCode::EQ);
			SM_REGISTER_NAME(OpCode::UNEQ);
			SM_REGISTER_NAME(OpCode::BIGGER);
			SM_REGISTER_NAME(OpCode::LESS);
			SM_REGISTER_NAME(OpCode::ADD);
			SM_REGISTER_NAME(OpCode::SUB);
			SM_REGISTER_NAME(OpCode::MULT);
			SM_REGISTER_NAME(OpCode::DIV);
			SM_REGISTER_NAME(OpCode::INCR);
			SM_REGISTER_NAME(OpCode::DECR);
			SM_REGISTER_NAME(OpCode::ASSIGN);

			// Interpreter
			SM_REGISTER_NAME(OpCode::END_PROGRAM);
			SM_REGISTER_NAME(OpCode::LOAD_LOCAL);
			SM_REGISTER_NAME(OpCode::LOAD_GLOBAL);
			SM_REGISTER_NAME(OpCode::STORE_LOCAL);
			SM_REGISTER_NAME(OpCode::STORE_GLOBAL);
			SM_REGISTER_NAME(OpCode::JUMP);
			SM_REGISTER_NAME(OpCode::JUMP_IF_NOT);
			SM_REGISTER_NAME(OpCode::CREATE_VARIABLE);
			SM_REGISTER_NAME(OpCode::POP);
		}
		return "";
	}
#undef SM_REGISTER_NAME

	template <typename T, typename... T2>
	std::string opCodeName(T opCode1, T2... opCodeRest) {
		static_assert(std::is_same<T, base::OpCode>::value);

		std::string out = opCodeName(opCode1);
		((out += ", " + opCodeName(opCodeRest)), ...);
		return out;
	}

#define SM_REGISTER_NAME(x, y) case x: return y
	inline std::string opCodeNameUser(OpCode opCode) {
		switch (opCode) {
			// Lexer
			SM_REGISTER_NAME(OpCode::TYPE, "type");
			SM_REGISTER_NAME(OpCode::END_STATEMENT, ";");
			SM_REGISTER_NAME(OpCode::COMMA, ",");
			SM_REGISTER_NAME(OpCode::IF, "<if>");
			SM_REGISTER_NAME(OpCode::ELSE, "<else>");
			SM_REGISTER_NAME(OpCode::WHILE, "<while>");
			SM_REGISTER_NAME(OpCode::CONTINUE, "<continue>");
			SM_REGISTER_NAME(OpCode::BREAK, "<break>");
			SM_REGISTER_NAME(OpCode::BRACKET_ROUND_OPEN, "(");
			SM_REGISTER_NAME(OpCode::BRACKET_ROUND_CLOSE, ")");
			SM_REGISTER_NAME(OpCode::BRACKET_SQUARE_OPEN, "[");
			SM_REGISTER_NAME(OpCode::BRACKET_SQUARE_CLOSE, "]");
			SM_REGISTER_NAME(OpCode::BRACKET_CURLY_OPEN, "{");
			SM_REGISTER_NAME(OpCode::BRACKET_CURLY_CLOSE, "}");
			SM_REGISTER_NAME(OpCode::NAME, "name");
			SM_REGISTER_NAME(OpCode::FUNC, "<func>");

			// Lexer + Interpreter
			SM_REGISTER_NAME(OpCode::LOAD_LITERAL, "literal");
			SM_REGISTER_NAME(OpCode::EQ, "==");
			SM_REGISTER_NAME(OpCode::UNEQ, "!=");
			SM_REGISTER_NAME(OpCode::BIGGER, ">");
			SM_REGISTER_NAME(OpCode::LESS, "<");
			SM_REGISTER_NAME(OpCode::ADD, "+");
			SM_REGISTER_NAME(OpCode::SUB, "-");
			SM_REGISTER_NAME(OpCode::MULT, "*");
			SM_REGISTER_NAME(OpCode::DIV, "/");
			SM_REGISTER_NAME(OpCode::INCR, "++");
			SM_REGISTER_NAME(OpCode::DECR, "--");
			SM_REGISTER_NAME(OpCode::ASSIGN, "=");
		}
		return "<error>";
	}
#undef SM_REGISTER_NAME

	template <typename T, typename... T2>
	std::string opCodeNameUser(T opCode1, T2... opCodeRest) {
		static_assert(std::is_same<T, base::OpCode>::value);

		std::string out = opCodeNameUser(opCode1);
		((out += ", " + opCodeNameUser(opCodeRest)), ...);
		return out;
	}
}