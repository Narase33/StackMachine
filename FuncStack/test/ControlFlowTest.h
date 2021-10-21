#pragma once

#include "catch.hpp"
#include "../src/Stackmachine/Stackmachine.h"
#include "../src/Parser/Parser.h"

using namespace base;
using namespace utils;
using namespace stackmachine;
using namespace compiler;

namespace controlFlowTest {
	void test_dataStack(std::string&& expression, std::initializer_list<BasicType> expected) {
		SECTION(expression) {
			try {
				Compiler compiler(std::move(expression));
				REQUIRE(compiler.isSuccessful());

				StackMachine machine(compiler.getProgram());
				INFO(machine.toString());

				machine.exec();
				std::stack<BasicType> dataStack = machine.getDataStack();
				REQUIRE(dataStack.size() == expected.size());

				for (const auto& expec : expected) {
					REQUIRE(dataStack.top().getInt() == expec.getInt());
					dataStack.pop();
				}
			} catch (const std::exception& e) {
				FAIL(e.what());
			}
		}
	}

	void test_variable(std::string&& expression, std::initializer_list<std::pair<std::string, BasicType>> expected) {
		SECTION(expression) {
			try {
				Compiler compiler(std::move(expression));
				REQUIRE(compiler.isSuccessful());

				StackMachine machine(compiler.getProgram());
				INFO(machine.toString());

				machine.exec();
				REQUIRE(machine.getDataStack().size() == 0);

				for (const auto& expec : expected) {
					REQUIRE((machine.get(expec.first) == expec.second).getBool());
				}
			} catch (const std::exception& e) {
				FAIL(e.what());
			}
		}
	}

	TEST_CASE("ControlFlow-Test-if") {
		// Basic
		test_dataStack("if ( true ) {2; }", { base::BasicType(2) });
		test_dataStack("if ( false ) { 2; }", { });
		test_dataStack("if ( true ) { 2; } else { 3; }", { base::BasicType(2) });
		test_dataStack("if ( false ) { 2; } else { 3; }", { base::BasicType(3) });

		// Complex condition
		test_dataStack("if ( 1 + 4 == 2 + 3 ) { 2; } else { 3; }", { base::BasicType(2) });
		test_dataStack("if ( 1 + 4 == 22 + 33 ) { 2; } else { 3; }", { base::BasicType(3) });

		// Complex negative condition
		test_dataStack("if ( 1 + 4 != 2 + 3 ) { 2; } else { 3; }", { base::BasicType(3) });
		test_dataStack("if ( 1 + 4 != 22 + 33 ) { 2; } else { 3; }", { base::BasicType(2) });

		// else if
		test_dataStack("if ( false ) { 1; } else if ( false ) { 2; } else if (false) {3;} else { 4; }", { base::BasicType(4) });

		test_dataStack("if ( false ) { 1; } else if ( false ) { 2; } else if (true) {3;} else { 4; }", { base::BasicType(3) });
		test_dataStack("if ( false ) { 1; } else if ( true ) { 2; } else if (false) {3;} else { 4; }", { base::BasicType(2) });
		test_dataStack("if ( true ) { 1; } else if ( false ) { 2; } else if (false) {3;} else { 4; }", { base::BasicType(1) });

		test_dataStack("if ( false ) { 1; } else if ( true ) { 2; } else if (true) {3;} else { 4; }", { base::BasicType(2) });
		test_dataStack("if ( true ) { 1; } else if ( true ) { 2; } else if (false) {3;} else { 4; }", { base::BasicType(1) });
		test_dataStack("if ( true ) { 1; } else if ( false ) { 2; } else if (true) {3;} else { 4; }", { base::BasicType(1) });

		test_dataStack("if ( true ) { 1; } else if ( true ) { 2; } else if (true) {3;} else { 4; }", { base::BasicType(1) });
	}

	TEST_CASE("ControlFlow-Test-while") {
		// Basic
		std::string code =
			"int i = 0;\n"
			"while (i < 10) {\n"
			"	i++;\n"
			"}";
		test_variable(std::move(code), { {"i", BasicType(10)} });

		// Two variables
		code =
			"int i = 0;\n"
			"int j = 0;\n"
			"while (i < 10) {\n"
			"	i++;\n"
			"	j = j + i;\n"
			"}";
		test_variable(std::move(code), { {"i", BasicType(10)}, {"j", BasicType(55)} });

		// Combined
		code =
			"int i = 0;\n"
			"int j = 0;\n"
			"while (i != 10) {\n"
			"	if (i == 5) {\n"
			"		j = 5;\n"
			"	}\n"
			"	i++;\n"
			"}";
		test_variable(std::move(code), { {"i", BasicType(10)}, {"j", BasicType(5)} });

		// Continue
		code =
			"int i = 0;\n"
			"int j = 0;\n"
			"while (i != 10) {\n"
			"	i++;\n"
			"	if (i > 5) {\n"
			"		continue;\n"
			"	}\n"
			"	j++;\n"
			"}";
		test_variable(std::move(code), { {"i", BasicType(10)}, {"j", BasicType(5)} });

		// Break
		code =
			"int i = 0;\n"
			"int j = 0;\n"
			"while (i != 10) {\n"
			"	i++;\n"
			"	j++;\n"
			"	if (i == 5) {\n"
			"		break;\n"
			"	}\n"
			"}";
		test_variable(std::move(code), { {"i", BasicType(5)}, {"j", BasicType(5)} });

		// Multi level
		code =
			"int i = 0;\n"
			"int k = 0;\n"
			"while (i < 5) {\n"
			"	int j = 0;\n"
			"	while (j < 5) {\n"
			"		k++;\n"
			"		j++;\n"
			"	}\n"
			"	i++;\n"
			"}";
		test_variable(std::move(code), { {"i", BasicType(5)}, {"k", BasicType(25)} });

		// Multi level break
		code =
			"int i = 0;\n"
			"int k = 0;\n"
			"while (i < 5) {\n"
			"	int j = 0;\n"
			"	while (j < 5) {\n"
			"		k++;\n"
			"		j++;\n"
			"		if (k == 8) {\n"
			"			break 2;\n"
			"		}\n"
			"	}\n"
			"	i++;\n"
			"}";
		test_variable(std::move(code), { {"i", BasicType(1)}, {"k", BasicType(8)} });

		// Multi level continue
		code =
			"int i = 0;\n"
			"int k = 0;\n"
			"while (i < 5) {\n"
			"	int j = 0;\n"
			"	i++;\n"
			"	while (j < 5) {\n"
			"		j++;\n"
			"		if (k == 8) {\n"
			"			continue 2;\n"
			"		}\n"
			"		k++;\n"
			"	}\n"
			"}";
		test_variable(std::move(code), { {"i", BasicType(5)}, {"k", BasicType(8)} });
	}
}