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
				std::list<base::StackFrame> programm = {
					StackFrame(Operator::LOAD, StackType(a)),
					StackFrame(Operator::LOAD, StackType(b)),
					StackFrame(op)
				};

				StackMachine machine(std::move(programm));

				INFO(machine.toString());

				const auto result = machine.exec();
				REQUIRE(result.has_value());
				REQUIRE((result.value() == expected).getBool());
			} catch (const std::exception & e) {
				FAIL(e.what());
			}
		}
	}
}

TEST_CASE("Operator-Test") {
	test(Operator::ADD, base::ValueType(6), base::ValueType(2), base::ValueType(8));
	test(Operator::SUB, base::ValueType(6), base::ValueType(2), base::ValueType(4));
	test(Operator::MULT, base::ValueType(6), base::ValueType(2), base::ValueType(12));
	test(Operator::DIV, base::ValueType(6), base::ValueType(2), base::ValueType(3));
}