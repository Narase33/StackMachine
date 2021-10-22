#pragma once

#include "catch.hpp"
#include "../src/Stackmachine/Stackmachine.h"
#include "../src/Parser/Parser.h"

using namespace base;
using namespace utils;
using namespace stackmachine;
using namespace compiler;
using namespace std::string_literals;

namespace variableTest {
	TEST_CASE("Variable-Test") {

		SECTION("Basic Tests") {
			{
				Compiler compiler("int value = 3;");
				REQUIRE(compiler.isSuccessful());

				StackMachine machine(compiler.getProgram());
				INFO(machine.toString());

				machine.exec();

				REQUIRE(machine.getVariable(0).getInt() == 3);
			}

			{
				Compiler compiler("uint value = 3u;");
				REQUIRE(compiler.isSuccessful());

				StackMachine machine(compiler.getProgram());
				INFO(machine.toString());

				machine.exec();
				REQUIRE(machine.getVariable(0).getUint() == 3);
			}

			{
				Compiler compiler("float value = 3.3;");
				REQUIRE(compiler.isSuccessful());

				StackMachine machine(compiler.getProgram());
				INFO(machine.toString());

				machine.exec();
				REQUIRE(machine.getVariable(0).getFloat() == 3.3);
			}

			{
				Compiler compiler("bool value = true;");
				REQUIRE(compiler.isSuccessful());

				StackMachine machine(compiler.getProgram());
				INFO(machine.toString());

				machine.exec();
				REQUIRE(machine.getVariable(0).getBool() == true);
			}

			{
				std::string code =
					"int a = 1;\n"
					"uint b = 2u;\n"
					"float c = 3.3;\n"
					"bool d = false;\n";

				Compiler compiler(std::move(code));
				REQUIRE(compiler.isSuccessful());

				StackMachine machine(compiler.getProgram());
				INFO(machine.toString());

				machine.exec();
				REQUIRE(machine.getVariable(0).getInt() == 1);
				REQUIRE(machine.getVariable(1).getUint() == 2);
				REQUIRE(machine.getVariable(2).getFloat() == 3.3);
				REQUIRE(machine.getVariable(3).getBool() == false);
			}
		}

		SECTION("Code") {
			std::string code =
				"int i = 1;"
				"i = 2;"
				"int j = i + 1;";

			Compiler compiler(std::move(code));
			REQUIRE(compiler.isSuccessful());

			StackMachine machine(compiler.getProgram());
			INFO(machine.toString());			machine.exec();

			REQUIRE(machine.getVariable(0).getInt() == 2);
			REQUIRE(machine.getVariable(1).getInt() == 3);
		}
	}
}
