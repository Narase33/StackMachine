#pragma once

#include "catch.hpp"
#include "../src/Stackmachine/Stackmachine.h"
#include "../src/Compiler/Compiler.h"

using namespace utils;
using namespace stackmachine;
using namespace compiler;

namespace parserTest {
	void test(std::string&& expression, base::BasicType expected) {
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
				//REQUIRE(machine.getDataStack().size() == 1);
				REQUIRE((machine.getGlobalVariable(0) == expected).getBool());
			} catch (const std::exception& e) {
				FAIL(e.what());
			}
		}
	}

	TEST_CASE("Parser-Test") {
		test("i = 6 + 2;", base::BasicType(6 + 2));
		test("i = 6+2;", base::BasicType(6 + 2));
		test("i = 6 - 2;", base::BasicType(6 - 2));
		test("i = 6-2;", base::BasicType(6 - 2));
		test("i = 6 * 2;", base::BasicType(6 * 2));
		test("i = 6*2;", base::BasicType(6 * 2));
		test("i = 6 / 2;", base::BasicType(6 / 2));
		test("i = 6/2;", base::BasicType(6 / 2));
		test("i = 3 ++;", base::BasicType(3 + 1));
		test("i = 3++;", base::BasicType(3 + 1));
		test("i = 3 --;", base::BasicType(3 - 1));
		test("i = 3--;", base::BasicType(3 - 1));

		test("i = 1 + 2 + 3;", base::BasicType(1 + 2 + 3));
		test("i = 12 - 2 - 3;", base::BasicType(12 - 2 - 3));
		test("i = 1 * 2 * 3;", base::BasicType(1 * 2 * 3));
		test("i = 12 / 2 / 3;", base::BasicType(12 / 2 / 3));

		test("i = 1 + 2 * 3;", base::BasicType(1 + 2 * 3));
		test("i = 1 + 2 * 3 + 4;", base::BasicType(1 + 2 * 3 + 4));
		test("i = 1 + 2 * 3 + 4 * 5;", base::BasicType(1 + 2 * 3 + 4 * 5));
		test("i = 1 + 2 * 3 + 4 * 5 + 6;", base::BasicType(1 + 2 * 3 + 4 * 5 + 6));

		test("i = 2 * ( 1 + 1 ) * 2;", base::BasicType(2 * (1 + 1) * 2));
		test("i = 2 * ( 1 + 1 ) * ( 2 + 2 ) * 3;", base::BasicType(2 * (1 + 1) * (2 + 2) * 3));
		test("i = 2 * ( ( 1 + 1 ) + ( 2 + 2 ) ) * 3;", base::BasicType(2 * ((1 + 1) + (2 + 2)) * 3));
	}
}