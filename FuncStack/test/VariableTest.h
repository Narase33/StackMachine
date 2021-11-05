#pragma once

#include "catch.hpp"
#include "../src/Stackmachine/Stackmachine.h"
#include "../src/Compiler/Compiler.h"

using namespace base;
using namespace stackmachine;
using namespace compiler;
using namespace std::string_literals;

namespace variableTest {
	TEST_CASE("Variable-Test") {
		const std::string main = "\nfunc main() {}";
		SECTION("Basic Tests") {
			{
				Compiler compiler("int value = 3;" + main);
				base::Program program = compiler.run();
				REQUIRE(compiler.isSuccess());

				StackMachine machine(std::move(program));
				INFO(machine.toString());

				machine.exec();
				INFO(machine.toString());
				REQUIRE(machine.getDataStack().size() == 1);
				REQUIRE(machine.getGlobalVariable(0).getInt() == 3);
			}

			{
				Compiler compiler("uint value = 3u;" + main);
				base::Program program = compiler.run();
				REQUIRE(compiler.isSuccess());

				StackMachine machine(std::move(program));
				INFO(machine.toString());

				machine.exec();
				INFO(machine.toString());
				REQUIRE(machine.getDataStack().size() == 1);
				REQUIRE(machine.getGlobalVariable(0).getUint() == 3);
			}

			{
				Compiler compiler("float value = 3.3;" + main);
				base::Program program = compiler.run();
				REQUIRE(compiler.isSuccess());

				StackMachine machine(std::move(program));
				INFO(machine.toString());

				machine.exec();
				INFO(machine.toString());
				REQUIRE(machine.getDataStack().size() == 1);
				REQUIRE(machine.getGlobalVariable(0).getFloat() == 3.3);
			}

			{
				Compiler compiler("bool value = true;" + main);
				base::Program program = compiler.run();
				REQUIRE(compiler.isSuccess());

				StackMachine machine(std::move(program));
				INFO(machine.toString());

				machine.exec();
				INFO(machine.toString());
				REQUIRE(machine.getDataStack().size() == 1);
				REQUIRE(machine.getGlobalVariable(0).getBool() == true);
			}

			{
				std::string code =
					"int a = 1;\n"
					"uint b = 2u;\n"
					"float c = 3.3;\n"
					"bool d = false;\n"
					+ main;

				Compiler compiler(std::move(code));
				base::Program program = compiler.run();
				REQUIRE(compiler.isSuccess());

				StackMachine machine(std::move(program));
				INFO(machine.toString());

				machine.exec();
				INFO(machine.toString());

				REQUIRE(machine.getDataStack().size() == 4);
				REQUIRE(machine.getGlobalVariable(0).getInt() == 1);
				REQUIRE(machine.getGlobalVariable(1).getUint() == 2);
				REQUIRE(machine.getGlobalVariable(2).getFloat() == 3.3);
				REQUIRE(machine.getGlobalVariable(3).getBool() == false);
			}
		}

		SECTION("Code") {
			std::string code =
				"int i = 1;\n"
				"int j = i + 1;\n"
				"func main() {\n"
				"	i = i + 2;\n"
				"}";

			Compiler compiler(std::move(code));
			base::Program program = compiler.run();
			REQUIRE(compiler.isSuccess());

			StackMachine machine(std::move(program));
			INFO(machine.toString());

			machine.exec();
			INFO(machine.toString());

			REQUIRE(machine.getDataStack().size() == 2);
			REQUIRE(machine.getGlobalVariable(0).getInt() == 3);
			REQUIRE(machine.getGlobalVariable(1).getInt() == 2);
		}
	}
}
