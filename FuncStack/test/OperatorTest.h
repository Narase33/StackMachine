#pragma once

#include "catch.hpp"
#include "../src/Stackmachine/Stackmachine.h"
#include "../src/Parser/Parser.h"

using namespace utils;
using namespace stackmachine;
using namespace compiler;

namespace operatorTest {
	void test(std::string&& expression, base::BasicType expected) {
		SECTION(expression) {
			try {
				Compiler compiler(std::move(expression));
				REQUIRE(compiler.isSuccessful());

				StackMachine machine(compiler.getProgram());
				INFO(machine.toString());
				machine.exec();

				//REQUIRE(machine.getDataStack().size() == 1);
				REQUIRE((machine.getVariable(0) == expected).getBool());
			} catch (const std::exception& e) {
				FAIL(e.what());
			}
		}
	}

	TEST_CASE("Operator-Test") {
		test("int i = 6 + 2;", base::BasicType(8));
		test("int i = 6 - 2;", base::BasicType(4));
		test("int i = 6 * 2;", base::BasicType(12));
		test("int i = 6 / 2;", base::BasicType(3));
		test("int i = 6++;", base::BasicType(7));
		test("int i = 6--;", base::BasicType(5));
	}
}