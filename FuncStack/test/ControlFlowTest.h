#pragma once

#include "catch.hpp"
#include "../src/Stackmachine/Stackmachine.h"
#include "../src/Parser/Parser.h"

using namespace base;
using namespace utils;
using namespace stackmachine;
using namespace compiler;

namespace controlFlowTest {
	void test_dataStack(std::string&& expression, BasicType expected) {
		SECTION(expression) {
			try {
				Compiler compiler(std::move(expression));
				REQUIRE(compiler.isSuccessful());

				StackMachine machine(compiler.getProgram());
				INFO(machine.toString());

				machine.exec();
				std::vector<BasicType> dataStack = machine.getDataStack();
				//REQUIRE(dataStack.size() == 1);

				REQUIRE(machine.getVariable(0).getInt() == expected.getInt());
				dataStack.pop_back();
			} catch (const std::exception& e) {
				FAIL(e.what());
			}
		}
	}

	void test_variable(std::string&& expression, std::vector<BasicType> expected) {
		SECTION(expression) {
			try {
				Compiler compiler(std::move(expression));
				REQUIRE(compiler.isSuccessful());

				StackMachine machine(compiler.getProgram());
				INFO(machine.toString());

				machine.exec();
				//REQUIRE(machine.getDataStack().size() == expected.size());

				for (int i = 0; i < expected.size(); i++) {
					REQUIRE((machine.getVariable(i) == expected[i]).getBool());
				}
			} catch (const std::exception& e) {
				FAIL(e.what());
			}
		}
	}

	void test_compileFail(std::string&& expression) {
		SECTION(expression) {
			REQUIRE_THROWS(Compiler(std::move(expression)));
		}
	}

	TEST_CASE("ControlFlow-Test-if") {
		// Basic
		test_dataStack("int i = 0; if ( true ) { i = 2; }", BasicType(2));
		test_dataStack("int i = 0; if ( false ) { i = 2; }", BasicType(0));
		test_dataStack("int i = 0; if ( true ) { i = 2; } else { i = 3; }", BasicType(2));
		test_dataStack("int i = 0; if ( false ) { i = 2; } else { i = 3; }", BasicType(3));

		// Complex condition
		test_dataStack("int i = 0; if ( 1 + 4 == 2 + 3 ) { i = 2; } else { i = 3; }", BasicType(2));
		test_dataStack("int i = 0; if ( 1 + 4 == 22 + 33 ) { i = 2; } else { i = 3; }", BasicType(3));

		// Complex negative condition
		test_dataStack("int i = 0; if ( 1 + 4 != 2 + 3 ) { i = 2; } else { i = 3; }", BasicType(3));
		test_dataStack("int i = 0; if ( 1 + 4 != 22 + 33 ) { i = 2; } else { i = 3; }", BasicType(2));

		// else if
		test_dataStack("int i = 0; if ( false ) { i = 1; } else if ( false ) { i = 2; } else if (false) {i = 3;} else { i = 4; }", BasicType(4));

		test_dataStack("int i = 0; if ( false ) { i = 1; } else if ( false ) { i = 2; } else if (true) {i = 3;} else { i = 4; }", BasicType(3));
		test_dataStack("int i = 0; if ( false ) { i = 1; } else if ( true ) { i = 2; } else if (false) {i = 3;} else { i = 4; }", BasicType(2));
		test_dataStack("int i = 0; if ( true ) { i = 1; } else if ( false ) { i = 2; } else if (false) {i = 3;} else { i = 4; }", BasicType(1));

		test_dataStack("int i = 0; if ( false ) { i = 1; } else if ( true ) { i = 2; } else if (true) {i = 3;} else { i = 4; }", BasicType(2));
		test_dataStack("int i = 0; if ( true ) { i = 1; } else if ( true ) { i = 2; } else if (false) {i = 3;} else { i = 4; }", BasicType(1));
		test_dataStack("int i = 0; if ( true ) { i = 1; } else if ( false ) { i = 2; } else if (true) {i = 3;} else { i = 4; }", BasicType(1));

		test_dataStack("int i = 0; if ( true ) { i = 1; } else if ( true ) { i = 2; } else if (true) {i = 3;} else { i = 4; }", BasicType(1));

		// Fails

		test_compileFail("int i = 0; { i = 1;} else {i=2;}");
	}

	TEST_CASE("ControlFlow-Test-while") {
		// Basic
		std::string code =
			"int i = 0;\n"
			"while (i < 10) {\n"
			"	i++;\n"
			"}";
		test_variable(std::move(code), { BasicType(10) });

		// Two variables
		code =
			"int i = 0;\n"
			"int j = 0;\n"
			"while (i < 10) {\n"
			"	i++;\n"
			"	j = j + i;\n"
			"}";
		test_variable(std::move(code), { BasicType(10), BasicType(55) });

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
		test_variable(std::move(code), { BasicType(10), BasicType(5) });

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
		test_variable(std::move(code), { BasicType(10), BasicType(5) });

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
		test_variable(std::move(code), { BasicType(5), BasicType(5) });

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
		test_variable(std::move(code), { BasicType(5), BasicType(25) });

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
		test_variable(std::move(code), { BasicType(1), BasicType(8) });

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
		test_variable(std::move(code), { BasicType(5), BasicType(8) });
	}
}