#pragma once

#include "catch.hpp"
#include "../src/Stackmachine/Stackmachine.h"
#include "../src/Compiler/Compiler.h"

using namespace stackmachine;
using namespace compiler;

namespace operatorTest {
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

	TEST_CASE("Operator-Test") {
		test("i = 6 + 2;", base::BasicType(8));
		test("i = 6 - 2;", base::BasicType(4));
		test("i = 6 * 2;", base::BasicType(12));
		test("i = 6 / 2;", base::BasicType(3));
		test("i = 6++;", base::BasicType(7));
		test("i = 6--;", base::BasicType(5));
	}
}