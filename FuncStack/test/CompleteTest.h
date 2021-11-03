#pragma once

#include "catch.hpp"
#include "../src/Stackmachine/Stackmachine.h"
#include "../src/Compiler/Compiler.h"

using namespace base;
using namespace stackmachine;
using namespace compiler;

namespace completeTest {
	void test(std::string&& code) {
		try {
			Compiler compiler(std::move(code));

			const auto compilerStart = std::chrono::steady_clock::now();
			Program program = compiler.run();
			std::cout << "Compile time:\t" << (std::chrono::steady_clock::now() - compilerStart) << std::endl;

			StackMachine machine(std::move(program));
			INFO(machine.toString());

			const auto machineStart = std::chrono::steady_clock::now();
			machine.exec();
			std::cout << "Exec time:\t" << (std::chrono::steady_clock::now() - machineStart) << std::endl;

			INFO(machine.toString());
			REQUIRE(machine.getDataStack().size() == 1);
			REQUIRE((machine.getGlobalVariable(0) == base::BasicType(3)).getBool());

			std::cout << machine.toString() << std::endl;
		} catch (const std::exception& e) {
			FAIL(e.what());
		}
	}

	TEST_CASE("Complete-Test") {
		std::string code = R"(
int i = 0;

func int add(int a, int b) {
	return a+b;
}

func main() {
	i = add(1, 2);
}
)";

		test(std::move(code));
	}
}