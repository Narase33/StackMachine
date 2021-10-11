#pragma once

#include "src/Exception.h"

enum class OpCode {
	// Global
	ERR, NOOP,

	// Parser only
	JUMP_LABEL, JUMP_LABEL_IF, LABEL,

	// Lexer
	END_STATEMENT,
	LITERAL,
	IF, ELSE,
	BRACKET_ROUND_OPEN, BRACKET_ROUND_CLOSE,
	TYPE,

	// Lexer + Interpreter
	BRACKET_CURLY_OPEN, BRACKET_CURLY_CLOSE,
	NAME,
	EQ, UNEQ,
	BIGGER, LESS,
	ADD, SUB,
	MULT, DIV,
	INCR, DECR,
	ASSIGN,

	// Interpreter
	END_PROGRAM,
	LOAD, STORE,
	JUMP, JUMP_IF,
	CREATE, POP,
};

OpCode opCodeFromKeyword(const std::string_view keyword) {
	if (keyword == "if") return OpCode::IF;
	if (keyword == "else") return OpCode::ELSE;
	return OpCode::ERR;
}

OpCode opCodeFromSymbol(const std::string_view symbol) {
	if (symbol.length() == 1) {
		switch (symbol[0]) {
			case '(': return OpCode::BRACKET_ROUND_OPEN;
			case ')': return OpCode::BRACKET_ROUND_CLOSE;
			case '{': return OpCode::BRACKET_CURLY_OPEN;
			case '}': return  OpCode::BRACKET_CURLY_CLOSE;
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

int opCodePriority(OpCode id) {
	switch (id) {
		case OpCode::LITERAL: return 0;
		case OpCode::NAME: return 0;
		case OpCode::BRACKET_ROUND_OPEN: return 0;
		case OpCode::BRACKET_CURLY_OPEN: return 0;
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

#define SM_REGISTER_NAME(x) case x: return #x
std::string opCodeName(OpCode id) {
	switch (id) {
		// Global
		SM_REGISTER_NAME(OpCode::NOOP);
		SM_REGISTER_NAME(OpCode::ERR);

		// Parser only
		SM_REGISTER_NAME(OpCode::JUMP_LABEL);
		SM_REGISTER_NAME(OpCode::JUMP_LABEL_IF);
		SM_REGISTER_NAME(OpCode::LABEL);

		// Lexer
		SM_REGISTER_NAME(OpCode::TYPE);
		SM_REGISTER_NAME(OpCode::END_STATEMENT);
		SM_REGISTER_NAME(OpCode::LITERAL);
		SM_REGISTER_NAME(OpCode::IF);
		SM_REGISTER_NAME(OpCode::ELSE);
		SM_REGISTER_NAME(OpCode::BRACKET_ROUND_OPEN);
		SM_REGISTER_NAME(OpCode::BRACKET_ROUND_CLOSE);

		// Lexer + Interpreter
		SM_REGISTER_NAME(OpCode::BRACKET_CURLY_OPEN);
		SM_REGISTER_NAME(OpCode::BRACKET_CURLY_CLOSE);
		SM_REGISTER_NAME(OpCode::NAME);
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
		SM_REGISTER_NAME(OpCode::LOAD);
		SM_REGISTER_NAME(OpCode::STORE);
		SM_REGISTER_NAME(OpCode::JUMP);
		SM_REGISTER_NAME(OpCode::JUMP_IF);
		SM_REGISTER_NAME(OpCode::CREATE);
		SM_REGISTER_NAME(OpCode::POP);
	}
	return "";
}
#undef SM_REGISTER_NAME