#pragma once

#include "catch.hpp"
#include "../src/Stackmachine/Stackmachine.h"
#include "../src/Compiler/Compiler.h"

using namespace base;
using namespace utils;
using namespace stackmachine;
using namespace compiler;

namespace controlFlowTest {
	void test_dataStack(std::string&& expression, BasicType expected) {
		SECTION(expression) {
			try {
				std::string code = "int i = 0;\n";
				code += "func main() {\n";
				code += expression + "\n";
				code += "}";

				Compiler compiler(std::move(code));
				REQUIRE(compiler.isSuccess());

				StackMachine machine(compiler.run());
				INFO(machine.toString());

				machine.exec();
				INFO(machine.toString());
				std::vector<BasicType> dataStack = machine.getDataStack();
				//REQUIRE(dataStack.size() == 1);

				REQUIRE(machine.getGlobalVariable(0).getInt() == expected.getInt());
				dataStack.pop_back();
			} catch (const std::exception& e) {
				FAIL(e.what());
			}
		}
	}

	void test_variable(std::string&& expression, std::vector<BasicType> expected) {
		SECTION(expression) {
			try {
				std::string code = "int i = 0;\n";
				code += "int j = 0;\n";
				code += "func main() {\n";
				code += expression + "\n";
				code += "}";

				Compiler compiler(std::move(code));
				REQUIRE(compiler.isSuccess());

				StackMachine machine(compiler.run());
				INFO(machine.toString());

				machine.exec();
				INFO(machine.toString());
				//REQUIRE(machine.getDataStack().size() == expected.size());

				for (int i = 0; i < expected.size(); i++) {
					REQUIRE((machine.getGlobalVariable(i) == expected[i]).getBool());
				}
			} catch (const std::exception& e) {
				FAIL(e.what());
			}
		}
	}

	void test_compileFail(std::string&& expression) {
		SECTION(expression) {
			Compiler compiler(std::move(expression));
			compiler.run();
			REQUIRE(!compiler.isSuccess());
		}
	}

	TEST_CASE("ControlFlow-Test-if") {
		// Basic
		test_dataStack("if ( true ) { i = 2; }", BasicType(2));
		test_dataStack("if ( false ) { i = 2; }", BasicType(0));
		test_dataStack("if ( true ) { i = 2; } else { i = 3; }", BasicType(2));
		test_dataStack("if ( false ) { i = 2; } else { i = 3; }", BasicType(3));

		// Complex condition
		test_dataStack("if ( 1 + 4 == 2 + 3 ) { i = 2; } else { i = 3; }", BasicType(2));
		test_dataStack("if ( 1 + 4 == 22 + 33 ) { i = 2; } else { i = 3; }", BasicType(3));

		// Complex negative condition
		test_dataStack("if ( 1 + 4 != 2 + 3 ) { i = 2; } else { i = 3; }", BasicType(3));
		test_dataStack("if ( 1 + 4 != 22 + 33 ) { i = 2; } else { i = 3; }", BasicType(2));

		// else if
		test_dataStack("if ( false ) { i = 1; } else if ( false ) { i = 2; } else if (false) {i = 3;} else { i = 4; }", BasicType(4));

		test_dataStack("if ( false ) { i = 1; } else if ( false ) { i = 2; } else if (true) {i = 3;} else { i = 4; }", BasicType(3));
		test_dataStack("if ( false ) { i = 1; } else if ( true ) { i = 2; } else if (false) {i = 3;} else { i = 4; }", BasicType(2));
		test_dataStack("if ( true ) { i = 1; } else if ( false ) { i = 2; } else if (false) {i = 3;} else { i = 4; }", BasicType(1));

		test_dataStack("if ( false ) { i = 1; } else if ( true ) { i = 2; } else if (true) {i = 3;} else { i = 4; }", BasicType(2));
		test_dataStack("if ( true ) { i = 1; } else if ( true ) { i = 2; } else if (false) {i = 3;} else { i = 4; }", BasicType(1));
		test_dataStack("if ( true ) { i = 1; } else if ( false ) { i = 2; } else if (true) {i = 3;} else { i = 4; }", BasicType(1));

		test_dataStack("if ( true ) { i = 1; } else if ( true ) { i = 2; } else if (true) {i = 3;} else { i = 4; }", BasicType(1));

		// Fails

		test_compileFail("{ i = 1;} else {i=2;}");
	}

	TEST_CASE("ControlFlow-Test-while") {
		// Basic
		std::string code =
			"while (i < 10) {\n"
			"	i++;\n"
			"}";
		test_variable(std::move(code), { BasicType(10) });

		// Two variables
		code =
			"while (i < 10) {\n"
			"	i++;\n"
			"	j = j + i;\n"
			"}";
		test_variable(std::move(code), { BasicType(10), BasicType(55) });

		// Combined
		code =
			"while (i != 10) {\n"
			"	if (i == 5) {\n"
			"		j = 5;\n"
			"	}\n"
			"	i++;\n"
			"}";
		test_variable(std::move(code), { BasicType(10), BasicType(5) });

		// Continue
		code =
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
			"while (i < 5) {\n"
			"	int k = 0;\n"
			"	while (k < 5) {\n"
			"		j++;\n"
			"		k++;\n"
			"	}\n"
			"	i++;\n"
			"}";
		test_variable(std::move(code), { BasicType(5), BasicType(25) });

		// Multi level break
		code =
			"while (i < 5) {\n"
			"	int k = 0;\n"
			"	while (k < 5) {\n"
			"		j++;\n"
			"		k++;\n"
			"		if (j == 8) {\n"
			"			break 2;\n"
			"		}\n"
			"	}\n"
			"	i++;\n"
			"}";
		test_variable(std::move(code), { BasicType(1), BasicType(8) });

		// Multi level continue
		code =
			"while (i < 5) {\n"
			"	int k = 0;\n"
			"	i++;\n"
			"	while (k < 5) {\n"
			"		k++;\n"
			"		if (j == 8) {\n"
			"			continue 2;\n"
			"		}\n"
			"		j++;\n"
			"	}\n"
			"}";
		test_variable(std::move(code), { BasicType(5), BasicType(8) });
	}
}