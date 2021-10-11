#pragma once

#include "catch.hpp"
#include "../src/Stackmachine/Stackmachine.h"

using namespace base;
using namespace utils;
using namespace stackmachine;

namespace {
	void test(OpCode op, base::BasicType a, base::BasicType b, base::BasicType expected) {
		const std::string name = a.toString() + " " + opCodeName(op) + " " + b.toString();
		SECTION(name) {
			try {
				const std::list<base::Operation> programm = {
					Operation(OpCode::LOAD, StackFrame(a)),
					Operation(OpCode::LOAD, StackFrame(b)),
					Operation(op)
				};

				StackMachine machine(programm);

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
	test(OpCode::ADD, base::BasicType(6), base::BasicType(2), base::BasicType(8));
	test(OpCode::SUB, base::BasicType(6), base::BasicType(2), base::BasicType(4));
	test(OpCode::MULT, base::BasicType(6), base::BasicType(2), base::BasicType(12));
	test(OpCode::DIV, base::BasicType(6), base::BasicType(2), base::BasicType(3));
}