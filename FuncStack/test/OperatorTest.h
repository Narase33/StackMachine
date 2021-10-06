#pragma once

#include "catch.hpp"
#include "src/Stackmachine/Stackmachine.h"

using namespace base;
using namespace utils;
using namespace stackmachine;

namespace {
	void test(Operator op, base::BasicType a, base::BasicType b, base::BasicType expected) {
		const std::string name = a.toString() + " " + getName(op) + " " + b.toString();
		SECTION(name) {
			try {
				std::list<base::Operation> programm = {
					Operation(Operator::LOAD, StackFrame(a)),
					Operation(Operator::LOAD, StackFrame(b)),
					Operation(op)
				};

				StackMachine machine(std::move(programm));

				INFO(machine.toString());

				const auto result = machine.exec();
				REQUIRE(result.has_value());
				REQUIRE((result.value().getValue() == expected).getBool());
			} catch (const std::exception & e) {
				FAIL(e.what());
			}
		}
	}
}

TEST_CASE("Operator-Test") {
	test(Operator::ADD, base::BasicType(6), base::BasicType(2), base::BasicType(8));
	test(Operator::SUB, base::BasicType(6), base::BasicType(2), base::BasicType(4));
	test(Operator::MULT, base::BasicType(6), base::BasicType(2), base::BasicType(12));
	test(Operator::DIV, base::BasicType(6), base::BasicType(2), base::BasicType(3));
}