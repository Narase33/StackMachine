#pragma once

#include "catch.hpp"
#include "../src/Stackmachine/Stackmachine.h"
#include "../src/Parser/Parser.h"

using namespace base;
using namespace utils;
using namespace stackmachine;
using namespace parser;

namespace {
	void test_dataStack(std::string&& expression, std::initializer_list<base::BasicType> expected) {
		SECTION(expression) {
			try {
				StackMachine machine(Compiler(std::move(expression)).getProgram());
				INFO(machine.toString());

				machine.exec();
				auto dataStack = machine.getDataStack();
				REQUIRE(dataStack.size() == expected.size());

				for (const auto& expec : expected) {
					REQUIRE(dataStack.top().getInt() == expec.getInt());
					dataStack.pop();
				}
			} catch (const std::exception& e) {
				FAIL(e.what());
			}
		}
	}
}

TEST_CASE("ControlFlow-Test") {
	// Basic
	test_dataStack("if ( true ) { 2; }", { base::BasicType(2) });
	test_dataStack("if ( false ) { 2; }", { });
	test_dataStack("if ( true ) { 2; } else { 3; }", { base::BasicType(2) });
	test_dataStack("if ( false ) { 2; } else { 3; }", { base::BasicType(3) });

	// Complex condition
	test_dataStack("if ( 1 + 4 == 2 + 3 ) { 2; } else { 3; }", { base::BasicType(2) });
	test_dataStack("if ( 1 + 4 == 22 + 33 ) { 2; } else { 3; }", { base::BasicType(3) });

	// Complex negative condition
	test_dataStack("if ( 1 + 4 != 2 + 3 ) { 2; } else { 3; }", { base::BasicType(3) });
	test_dataStack("if ( 1 + 4 != 22 + 33 ) { 2; } else { 3; }", { base::BasicType(2) });

	// else if
	test_dataStack("if ( false ) { 1; } else if ( false ) { 2 } else if (false) {3;} else { 4; }", { base::BasicType(4) });

	test_dataStack("if ( false ) { 1; } else if ( false ) { 2; } else if (true) {3;} else { 4; }", { base::BasicType(3) });
	test_dataStack("if ( false ) { 1; } else if ( true ) { 2; } else if (false) {3;} else { 4; }", { base::BasicType(2) });
	test_dataStack("if ( true ) { 1; } else if ( false ) { 2; } else if (false) {3;} else { 4; }", { base::BasicType(1) });

	test_dataStack("if ( false ) { 1; } else if ( true ) { 2; } else if (true) {3;} else { 4; }", { base::BasicType(2) });
	test_dataStack("if ( true ) { 1; } else if ( true ) { 2; } else if (false) {3;} else { 4; }", { base::BasicType(1) });
	test_dataStack("if ( true ) { 1; } else if ( false ) { 2; } else if (true) {3;} else { 4; }", { base::BasicType(1) });

	test_dataStack("if ( true ) { 1; } else if ( true ) { 2; } else if (true) {3;} else { 4; }", { base::BasicType(1) });
}