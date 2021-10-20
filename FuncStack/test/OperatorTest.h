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

				const auto result = machine.exec();
				REQUIRE(result.has_value());
				REQUIRE((result.value().getValue() == expected).getBool());
			} catch (const std::exception& e) {
				FAIL(e.what());
			}
		}
	}

	TEST_CASE("Operator-Test") {
		test("6 + 2;", base::BasicType(8));
		test("6 - 2;", base::BasicType(4));
		test("6 * 2;", base::BasicType(12));
		test("6 / 2;", base::BasicType(3));
		test("6++;", base::BasicType(7));
		test("6--;", base::BasicType(5));
	}
}