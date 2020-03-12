#pragma once

#include "catch.hpp"
#include "src/Stackmachine/Stackmachine.h"

using namespace base;
using namespace utils;
using namespace stackmachine;

namespace {
	void test(Operator op, base::ValueType a, base::ValueType b, base::ValueType expected) {
		const std::string name = a.toString() + " " + getName(op) + " " + b.toString();
		SECTION(name) {
			try {
				StackMachine machine;

				machine.add(op);
				machine.add(Operator::LOAD, b);
				machine.add(Operator::LOAD, a);

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

TEST_CASE("Operator-Test") {
	test(Operator::ADD, 6, 2, 8);
	test(Operator::SUB, 6, 2, 4);
	test(Operator::MULT, 6, 2, 12);
	test(Operator::DIV, 6, 2, 3);
}