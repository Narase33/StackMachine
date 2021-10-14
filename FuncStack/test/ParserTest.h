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
				StackMachine machine(Compiler(std::move(expression)).getProgram());
				INFO(machine.toString());

				const auto result = machine.exec();
				REQUIRE(result.has_value());
				REQUIRE((result.value().getValue() == expected).getBool());
			} catch (const std::exception& e) {
				FAIL(e.what());
			}
		}
	}

	TEST_CASE("Parser-Test") {
		test("6 + 2;", base::BasicType(6 + 2));
		test("6+2;", base::BasicType(6 + 2));
		test("6 - 2;", base::BasicType(6 - 2));
		test("6-2;", base::BasicType(6 - 2));
		test("6 * 2;", base::BasicType(6 * 2));
		test("6*2;", base::BasicType(6 * 2));
		test("6 / 2;", base::BasicType(6 / 2));
		test("6/2;", base::BasicType(6 / 2));
		test("3 ++;", base::BasicType(3 + 1));
		test("3++;", base::BasicType(3 + 1));
		test("3 --;", base::BasicType(3 - 1));
		test("3--;", base::BasicType(3 - 1));

		test("1 + 2 + 3;", base::BasicType(1 + 2 + 3));
		test("12 - 2 - 3;", base::BasicType(12 - 2 - 3));
		test("1 * 2 * 3;", base::BasicType(1 * 2 * 3));
		test("12 / 2 / 3;", base::BasicType(12 / 2 / 3));

		test("1 + 2 * 3;", base::BasicType(1 + 2 * 3));
		test("1 + 2 * 3 + 4;", base::BasicType(1 + 2 * 3 + 4));
		test("1 + 2 * 3 + 4 * 5;", base::BasicType(1 + 2 * 3 + 4 * 5));
		test("1 + 2 * 3 + 4 * 5 + 6;", base::BasicType(1 + 2 * 3 + 4 * 5 + 6));

		test("2 * ( 1 + 1 ) * 2;", base::BasicType(2 * (1 + 1) * 2));
		test("2 * ( 1 + 1 ) * ( 2 + 2 ) * 3;", base::BasicType(2 * (1 + 1) * (2 + 2) * 3));
		test("2 * ( ( 1 + 1 ) + ( 2 + 2 ) ) * 3;", base::BasicType(2 * ((1 + 1) + (2 + 2)) * 3));
	}
}