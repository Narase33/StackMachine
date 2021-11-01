#pragma once

#include "catch.hpp"
#include "../src/Stackmachine/Stackmachine.h"
#include "../src/Compiler/Compiler.h"

using namespace base;
using namespace stackmachine;
using namespace compiler;

namespace ´functionTest {
	TEST_CASE("Function-Test") {
		std::string code = R"(
int i = 0;
int j = 0;
int k = 0;

func sub2() {
	k = i + 3;
}

func sub1() {
	j = i + 2;
	sub2();
}

func main() {
	i = i + 1;
	sub1();
}
)";

		try {
			Compiler compiler(std::move(code));
			REQUIRE(compiler.isSuccess());

			StackMachine machine(compiler.run());
			INFO(machine.toString());
			machine.exec();

			INFO(machine.toString());
			//REQUIRE(machine.getDataStack().size() == 1);
			REQUIRE((machine.getGlobalVariable(0) == base::BasicType(1)).getBool());
			REQUIRE((machine.getGlobalVariable(1) == base::BasicType(3)).getBool());
			REQUIRE((machine.getGlobalVariable(2) == base::BasicType(4)).getBool());
		} catch (const std::exception& e) {
			FAIL(e.what());
		}
	}
}