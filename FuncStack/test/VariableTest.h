#pragma once

#include "catch.hpp"
#include "src/Stackmachine/Stackmachine.h"
#include "src/Parser/Parser.h"

using namespace base;
using namespace utils;
using namespace stackmachine;
using namespace parser;
using namespace std::string_literals;

TEST_CASE("Variable-Test") {
	
	SECTION("Load") {
		StackMachine machine(std::list<StackFrame>{StackFrame(Operator::LOAD, StackType("a"s))});
		machine.set("a", ValueType(3.0));

		REQUIRE((machine.exec().value() == ValueType(3.0)).getBool());
	}

	SECTION("Store") {
		std::list<StackFrame> programm;

		{
			programm.push_back(StackFrame(Operator::LOAD, StackType(ValueType(4.0))));
			StackMachine machine(programm);
			REQUIRE((machine.exec().value() == ValueType(4.0)).getBool());
		}

		{
			programm.push_back(StackFrame(Operator::STORE, StackType("b"s)));
			StackMachine machine(programm);
			REQUIRE(!machine.exec().has_value());
		}

		{
			programm.push_back(StackFrame(Operator::LOAD, StackType("b"s)));
			StackMachine machine(programm);
			REQUIRE((machine.exec().value() == ValueType(4.0)).getBool());
		}

		{
			programm.push_back(StackFrame(Operator::STORE, StackType("c"s)));
			StackMachine machine(programm);
			REQUIRE(!machine.exec().has_value());
		}

		{
			programm.push_back(StackFrame(Operator::LOAD, StackType("c"s)));
			StackMachine machine(programm);
			REQUIRE((machine.exec().value() == ValueType(4.0)).getBool());
		}

		{
			programm.push_back(StackFrame(Operator::POP));
			programm.push_back(StackFrame(Operator::LOAD, StackType("b"s)));
			StackMachine machine(programm);
			REQUIRE((machine.exec().value() == ValueType(4.0)).getBool());
		}

		{
			programm.push_back(StackFrame(Operator::POP));
			StackMachine machine(programm);
			REQUIRE(!machine.exec().has_value());
		}

		{
			programm.push_back(StackFrame(Operator::LOAD, StackType(ValueType(5.0))));
			programm.push_back(StackFrame(Operator::STORE, StackType("b"s)));
			StackMachine machine(programm);
			REQUIRE(!machine.exec().has_value());
		}

		{
			programm.push_back(StackFrame(Operator::LOAD, StackType("b"s)));
			StackMachine machine(programm);
			REQUIRE((machine.exec().value() == base::ValueType(5.0)).getBool());
		}
	}
}