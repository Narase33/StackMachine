#pragma once

#include "catch.hpp"
#include "../src/Stackmachine/Stackmachine.h"
#include "../src/Parser/Parser.h"

using namespace base;
using namespace utils;
using namespace stackmachine;
using namespace parser;
using namespace std::string_literals;

TEST_CASE("Variable-Test") {

	SECTION("Basic Tests") {
		{
			StackMachine machine(Compiler(std::move("int value = 3;")).getProgram());
			machine.exec();
			REQUIRE(machine.get("value").getInt() == 3);
		}

		{
			StackMachine machine(Compiler(std::move("uint value = 3u;")).getProgram());
			machine.exec();
			REQUIRE(machine.get("value").getUint() == 3);
		}

		{
			StackMachine machine(Compiler(std::move("float value = 3.3;")).getProgram());
			machine.exec();
			REQUIRE(machine.get("value").getFloat() == 3.3);
		}

		{
			StackMachine machine(Compiler(std::move("bool value = true;")).getProgram());
			machine.exec();
			REQUIRE(machine.get("value").getBool() == true);
		}

		{
			std::string code =
				"int a = 1;\n"
				"uint b = 2u;\n"
				"float c = 3.3;\n"
				"bool d = false;\n";
			StackMachine machine(Compiler(std::move(code)).getProgram());
			machine.exec();
			REQUIRE(machine.get("a").getInt() == 1);
			REQUIRE(machine.get("b").getUint() == 2);
			REQUIRE(machine.get("c").getFloat() == 3.3);
			REQUIRE(machine.get("d").getBool() == false);
		}
	}

	SECTION("Code") {
		std::string code =
			"int i = 1;"
			"i = 2;"
			"int j = i + 1;";

		StackMachine machine(Compiler(std::move(code)).getProgram());
		machine.exec();

		REQUIRE(machine.get("i").getInt() == 2);
		REQUIRE(machine.get("j").getInt() == 3);
	}
}
