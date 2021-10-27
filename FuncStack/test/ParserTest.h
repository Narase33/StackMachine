#pragma once

#include "catch.hpp"
#include "../src/Stackmachine/Stackmachine.h"
#include "../src/Parser/Parser.h"

using namespace utils;
using namespace stackmachine;
using namespace compiler;

namespace parserTest {
	void test(std::string&& expression, base::BasicType expected) {
		SECTION(expression) {
			try {
				Compiler compiler(std::move(expression));
				REQUIRE(compiler.isSuccessful());

				StackMachine machine(compiler.getProgram());
				INFO(machine.toString());
				machine.exec();

				INFO(machine.toString());
				//REQUIRE(machine.getDataStack().size() == 1);
				REQUIRE((machine.getVariable(0) == expected).getBool());
			} catch (const std::exception& e) {
				FAIL(e.what());
			}
		}
	}

	TEST_CASE("Parser-Test") {
		test("int i = 6 + 2;", base::BasicType(6 + 2));
		test("int i = 6+2;", base::BasicType(6 + 2));
		test("int i = 6 - 2;", base::BasicType(6 - 2));
		test("int i = 6-2;", base::BasicType(6 - 2));
		test("int i = 6 * 2;", base::BasicType(6 * 2));
		test("int i = 6*2;", base::BasicType(6 * 2));
		test("int i = 6 / 2;", base::BasicType(6 / 2));
		test("int i = 6/2;", base::BasicType(6 / 2));
		test("int i = 3 ++;", base::BasicType(3 + 1));
		test("int i = 3++;", base::BasicType(3 + 1));
		test("int i = 3 --;", base::BasicType(3 - 1));
		test("int i = 3--;", base::BasicType(3 - 1));

		test("int i = 1 + 2 + 3;", base::BasicType(1 + 2 + 3));
		test("int i = 12 - 2 - 3;", base::BasicType(12 - 2 - 3));
		test("int i = 1 * 2 * 3;", base::BasicType(1 * 2 * 3));
		test("int i = 12 / 2 / 3;", base::BasicType(12 / 2 / 3));

		test("int i = 1 + 2 * 3;", base::BasicType(1 + 2 * 3));
		test("int i = 1 + 2 * 3 + 4;", base::BasicType(1 + 2 * 3 + 4));
		test("int i = 1 + 2 * 3 + 4 * 5;", base::BasicType(1 + 2 * 3 + 4 * 5));
		test("int i = 1 + 2 * 3 + 4 * 5 + 6;", base::BasicType(1 + 2 * 3 + 4 * 5 + 6));

		test("int i = 2 * ( 1 + 1 ) * 2;", base::BasicType(2 * (1 + 1) * 2));
		test("int i = 2 * ( 1 + 1 ) * ( 2 + 2 ) * 3;", base::BasicType(2 * (1 + 1) * (2 + 2) * 3));
		test("int i = 2 * ( ( 1 + 1 ) + ( 2 + 2 ) ) * 3;", base::BasicType(2 * ((1 + 1) + (2 + 2)) * 3));
	}
}