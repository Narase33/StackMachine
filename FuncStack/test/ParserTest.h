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
				auto programm = parser::parse(std::forward<std::string>(expression));

				StackMachine machine(std::move(programm));
				INFO(machine.toString());

				const auto result = machine.exec();
				REQUIRE(result.has_value());
				REQUIRE((result.value() == expected).getBool());
			} catch (const std::exception& e) {
				FAIL(e.what());
			}
		}
	}
}

TEST_CASE("Parser-Test") {
	test("6 + 2", base::ValueType(6 + 2));
	test("6+2", base::ValueType(6 + 2));
	test("6 - 2", base::ValueType(6 - 2));
	test("6-2", base::ValueType(6 - 2));
	test("6 * 2", base::ValueType(6 * 2));
	test("6*2", base::ValueType(6 * 2));
	test("6 / 2", base::ValueType(6 / 2));
	test("6/2", base::ValueType(6 / 2));
	test("3 ++", base::ValueType(3 + 1));
	test("3++", base::ValueType(3 + 1));
	test("3 --", base::ValueType(3 - 1));
	test("3--", base::ValueType(3 - 1));

	test("1 + 2 + 3", base::ValueType(1 + 2 + 3));
	test("12 - 2 - 3", base::ValueType(12 - 2 - 3));
	test("1 * 2 * 3", base::ValueType(1 * 2 * 3));
	test("12 / 2 / 3", base::ValueType(12 / 2 / 3));

	test("1 + 2 * 3", base::ValueType(1 + 2 * 3));
	test("1 + 2 * 3 + 4", base::ValueType(1 + 2 * 3 + 4));
	test("1 + 2 * 3 + 4 * 5", base::ValueType(1 + 2 * 3 + 4 * 5));
	test("1 + 2 * 3 + 4 * 5 + 6", base::ValueType(1 + 2 * 3 + 4 * 5 + 6));

	test("2 * ( 1 + 1 ) * 2", base::ValueType(2 * (1 + 1) * 2));
	test("2 * ( 1 + 1 ) * ( 2 + 2 ) * 3", base::ValueType(2 * (1 + 1) * (2 + 2) * 3));
	test("2 * ( ( 1 + 1 ) + ( 2 + 2 ) ) * 3", base::ValueType(2 * ((1 + 1) + (2 + 2)) * 3));

	test("2 * { 1 + 1 } * 2", base::ValueType(2 * (1 + 1) * 2));
	test("2 * { 1 + 1 } * { 2 + 2 } * 3", base::ValueType(2 * (1 + 1) * (2 + 2) * 3));
	test("2 * { { 1 + 1 } + { 2 + 2 } } * 3", base::ValueType(2 * ((1 + 1) + (2 + 2)) * 3));
}