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

	/*SECTION("Load") {
		std::list<Operation> program{
			Operation(Operator::LOAD, StackFrame(BasicType(3.0))),
			Operation(Operator::LOAD, StackFrame(BasicType(BasicType::stringToId("int")))),
			Operation(Operator::LOAD, StackFrame("a"s));
			Operation(Operator::CREATE),
			Operation(Operator::)
			Operation(Operator::LOAD, StackFrame("a"s))
		};

		machine.add("a", BasicType(3.0));

		StackMachine machine(

		REQUIRE((machine.exec().value().getValue() == BasicType(3.0)).getBool());
	}

	SECTION("Store") {
		std::list<Operation> programm;

		{
			programm.push_back(Operation(Operator::LOAD, StackFrame(BasicType(4.0))));
			StackMachine machine(programm);
			REQUIRE((machine.exec().value().getValue() == BasicType(4.0)).getBool());
		}

		{
			programm.push_back(Operation(Operator::CREATE, StackFrame(BasicType(BasicType::stringToId("float"))), StackFrame("b"s)));
			programm.push_back(Operation(Operator::STORE, StackFrame("b"s)));
			StackMachine machine(programm);
			REQUIRE(!machine.exec().has_value());
		}

		{
			programm.push_back(Operation(Operator::LOAD, StackFrame("b"s)));
			StackMachine machine(programm);
			REQUIRE((machine.exec().value().getValue() == BasicType(4.0)).getBool());
		}

		{
			programm.push_back(Operation(Operator::CREATE, StackFrame(BasicType(BasicType::stringToId("float"))), StackFrame("c"s)));
			programm.push_back(Operation(Operator::STORE, StackFrame("c"s)));
			StackMachine machine(programm);
			REQUIRE(!machine.exec().has_value());
		}

		{
			programm.push_back(Operation(Operator::LOAD, StackFrame("c"s)));
			StackMachine machine(programm);
			REQUIRE((machine.exec().value().getValue() == BasicType(4.0)).getBool());
		}

		{
			programm.push_back(Operation(Operator::POP));
			programm.push_back(Operation(Operator::LOAD, StackFrame("b"s)));
			StackMachine machine(programm);
			REQUIRE((machine.exec().value().getValue() == BasicType(4.0)).getBool());
		}

		{
			programm.push_back(Operation(Operator::POP));
			StackMachine machine(programm);
			REQUIRE(!machine.exec().has_value());
		}

		{
			programm.push_back(Operation(Operator::LOAD, StackFrame(BasicType(5.0))));
			programm.push_back(Operation(Operator::STORE, StackFrame("b"s)));
			StackMachine machine(programm);
			REQUIRE(!machine.exec().has_value());
		}

		{
			programm.push_back(Operation(Operator::LOAD, StackFrame("b"s)));
			StackMachine machine(programm);
			REQUIRE((machine.exec().value().getValue() == base::BasicType(5.0)).getBool());
		}
	}

	SECTION("Scope") {
		std::list<Operation> programm;

		{
			programm.push_back(Operation(Operator::LOAD, StackFrame(BasicType(4.0))));
			StackMachine machine(programm);
			REQUIRE((machine.exec().value().getValue() == BasicType(4.0)).getBool());
		}

		{
			programm.push_back(Operation(Operator::BRACKET_OPEN));
			programm.push_back(Operation(Operator::CREATE, StackFrame(BasicType(BasicType::stringToId("float"))), StackFrame("b"s)));
			programm.push_back(Operation(Operator::STORE, StackFrame("b"s)));
			StackMachine machine(programm);
			REQUIRE(!machine.exec().has_value());
		}

		{
			programm.push_back(Operation(Operator::LOAD, StackFrame("b"s)));
			StackMachine machine(programm);
			REQUIRE((machine.exec().value().getValue() == BasicType(4.0)).getBool());
		}

		{
			programm.push_back(Operation(Operator::BRACKET_CLOSE));
			programm.push_back(Operation(Operator::LOAD, StackFrame("b"s)));
			StackMachine machine(programm);
			REQUIRE_THROWS((machine.exec().value().getValue() == BasicType(4.0)).getBool());
		}
	}*/

	SECTION("Code") {
		std::string code = R"(
int i = 1
i = 2
int j = i + 1
)";

		auto programm = parser::parse(std::forward<std::string>(code));
		StackMachine machine(std::move(programm));
	}
}
