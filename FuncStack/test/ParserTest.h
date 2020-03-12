#pragma once

#include "catch.hpp"
#include "src/Stackmachine/Stackmachine.h"
#include "src/Parser/Parser.h"

using namespace base;
using namespace utils;
using namespace stackmachine;
using namespace parser;

namespace {
	void test(std::string&& expression, base::ValueType expected) {
		SECTION(expression) {
			try {
				auto stack = parser::parse(std::forward<std::string>(expression));

				StackMachine machine;
				machine.add(stack);
				INFO(machine.toString());

				const auto result = machine.exec();
				REQUIRE(result.has_value());
				REQUIRE(result.value() == expected);
			} catch (const std::exception & e) {
				FAIL(e.what());
			}
		}
	}
}

TEST_CASE("Parser-Test") {
	test("6 + 2", 6 + 2);
	test("6+2", 6 + 2);
	test("6 - 2", 6 - 2);
	test("6-2", 6 - 2);
	test("6 * 2", 6 * 2);
	test("6*2", 6 * 2);
	test("6 / 2", 6 / 2);
	test("6/2", 6 / 2);
	test("3 ++", 3 + 1);
	test("3++", 3 + 1);
	test("3 --", 3 - 1);
	test("3--", 3 - 1);

	test("1 + 2 + 3", 1 + 2 + 3);
	test("12 - 2 - 3", 12 - 2 - 3);
	test("1 * 2 * 3", 1 * 2 * 3);
	test("12 / 2 / 3", 12 / 2 / 3);

	test("1 + 2 * 3", 1 + 2 * 3);
	test("1 + 2 * 3 + 4", 1 + 2 * 3 + 4);
	test("1 + 2 * 3 + 4 * 5", 1 + 2 * 3 + 4 * 5);
	test("1 + 2 * 3 + 4 * 5 + 6", 1 + 2 * 3 + 4 * 5 + 6);

	test("2 * ( 1 + 1 ) * 2", 2 * (1 + 1) * 2);
	test("2 * ( 1 + 1 ) * ( 2 + 2 ) * 3", 2 * (1 + 1) * (2 + 2) * 3);
	test("2 * ( ( 1 + 1 ) + ( 2 + 2 ) ) * 3", 2 * ((1 + 1) + (2 + 2)) * 3);

	test("2 * { 1 + 1 } * 2", 2 * (1 + 1) * 2);
	test("2 * { 1 + 1 } * { 2 + 2 } * 3", 2 * (1 + 1) * (2 + 2) * 3);
	test("2 * { { 1 + 1 } + { 2 + 2 } } * 3", 2 * ((1 + 1) + (2 + 2)) * 3);
}