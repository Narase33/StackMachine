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
	StackMachine machine;

	SECTION("Load") {
		REQUIRE(machine.size() == 0);

		machine.add(Operator::LOAD, "a"s);
		machine.set("a", 3.0);

		REQUIRE(machine.exec().value() == 3.0);
	}

	SECTION("Store") {
		machine.add(Operator::LOAD, 4.0);
		REQUIRE(machine.exec().value() == 4.0);


		machine.add(Operator::STORE, "b"s);
		REQUIRE(!machine.exec().has_value());

		machine.add(Operator::LOAD, "b"s);
		REQUIRE(machine.exec().value() == 4.0);


		machine.add(Operator::STORE, "c"s);
		REQUIRE(!machine.exec().has_value());

		machine.add(Operator::LOAD, "c"s);
		REQUIRE(machine.exec().value() == 4.0);

		machine.add(Operator::LOAD, "b"s);
		machine.add(Operator::POP);
		REQUIRE(machine.exec().value() == 4.0);


		machine.add(Operator::POP);
		REQUIRE(!machine.exec().has_value());

		machine.add(Operator::STORE, "b"s);
		machine.add(Operator::LOAD, 5.0);
		REQUIRE(!machine.exec().has_value());

		machine.add(Operator::LOAD, "b"s);
		REQUIRE(machine.exec().value() == 5.0);
	}
}