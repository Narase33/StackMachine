#pragma once

#include "catch.hpp"
#include "../src/Stackmachine/Stackmachine.h"
#include "../src/Compiler/Compiler.h"

using namespace stackmachine;
using namespace compiler;
using namespace base;

namespace tokenizerTest {
	void test(const std::string& expression, OpCode expected) {
		SECTION(expression) {
			try {
				Tokenizer tokenizer(expression);
				REQUIRE(tokenizer.next().opCode == expected);
			} catch (const std::exception& e) {
				FAIL(e.what());
			}
		}

		SECTION(expression + " with hint") {
			try {
				Tokenizer tokenizer(expression);
				REQUIRE(tokenizer.next(expected).opCode == expected);
			} catch (const std::exception& e) {
				FAIL(e.what());
			}
		}
	}

#define SM_CREATE_TEST(x) test(opCodeName(x).str, x);
	TEST_CASE("Tokenizer-Test") {
		test("int", OpCode::TYPE);
		test("abc", OpCode::NAME);
		test("123", OpCode::LOAD_LITERAL);
		test("123u", OpCode::LOAD_LITERAL);
		test("1.23", OpCode::LOAD_LITERAL);
		test("true", OpCode::LOAD_LITERAL);
		test("false", OpCode::LOAD_LITERAL);
		SM_CREATE_TEST(OpCode::END_STATEMENT);
		SM_CREATE_TEST(OpCode::COMMA);
		SM_CREATE_TEST(OpCode::IF);
		SM_CREATE_TEST(OpCode::ELSE);
		SM_CREATE_TEST(OpCode::WHILE);
		SM_CREATE_TEST(OpCode::CONTINUE);
		SM_CREATE_TEST(OpCode::BREAK);
		SM_CREATE_TEST(OpCode::BRACKET_ROUND_OPEN);
		SM_CREATE_TEST(OpCode::BRACKET_ROUND_CLOSE);
		SM_CREATE_TEST(OpCode::BRACKET_SQUARE_OPEN);
		SM_CREATE_TEST(OpCode::BRACKET_SQUARE_CLOSE);
		SM_CREATE_TEST(OpCode::BRACKET_CURLY_OPEN);
		SM_CREATE_TEST(OpCode::BRACKET_CURLY_CLOSE);
		SM_CREATE_TEST(OpCode::FUNC);
		SM_CREATE_TEST(OpCode::RETURN);
		SM_CREATE_TEST(OpCode::EQ);
		SM_CREATE_TEST(OpCode::UNEQ);
		SM_CREATE_TEST(OpCode::BIGGER);
		SM_CREATE_TEST(OpCode::LESS);
		SM_CREATE_TEST(OpCode::ADD);
		SM_CREATE_TEST(OpCode::SUB);
		SM_CREATE_TEST(OpCode::MULT);
		SM_CREATE_TEST(OpCode::DIV);
		SM_CREATE_TEST(OpCode::INCR);
		SM_CREATE_TEST(OpCode::DECR);
		SM_CREATE_TEST(OpCode::ASSIGN);
	}
}
#undef SM_CREATE_TEST