#pragma once

#include "catch.hpp"
#include "src/Stackmachine/Stackmachine.h"
#include "src/Parser/Parser.h"

using namespace base;
using namespace utils;
using namespace stackmachine;
using namespace parser;

namespace {
	void test_dataStack(std::string&& expression, std::initializer_list<base::StackType> expected) {
		SECTION(expression) {
			try {
				auto stack = parser::parse(std::forward<std::string>(expression));

				StackMachine machine;
				machine.add(stack);
				INFO(machine.toString());

				machine.exec();
				auto dataStack = machine.getDataStack();
				REQUIRE(dataStack.size() == expected.size());

				for (const auto& expec : expected) {
					REQUIRE(dataStack.top().get<ValueType>().get<long>() == expec.get<ValueType>().get<long>());
					dataStack.pop();
				}
			} catch (const std::exception & e) {
				FAIL(e.what());
			}
		}
	}
}

TEST_CASE("ControlFlow-Test") {
	// Basic
	test_dataStack("if ( true ) { 2 } else { 3 }", std::initializer_list<base::StackType>{ 2 });
	test_dataStack("if ( false ) { 2 } else { 3 }", std::initializer_list<base::StackType>{ 3 });

	// Complex condition
	test_dataStack("if ( 1 + 4 == 2 + 3 ) { 2 } else { 3 }", std::initializer_list<base::StackType>{ 2 });
	test_dataStack("if ( 1 + 4 == 22 + 33 ) { 2 } else { 3 }", std::initializer_list<base::StackType>{ 3 });

	// Complex negative condition
	test_dataStack("if ( 1 + 4 != 2 + 3 ) { 2 } else { 3 }", std::initializer_list<base::StackType>{ 3 });
	test_dataStack("if ( 1 + 4 != 22 + 33 ) { 2 } else { 3 }", std::initializer_list<base::StackType>{ 2 });

	// Complex all
	test_dataStack("1 + if ( 1 + 4 == 2 + 3 ) { 2 } else { 3 } + 4", std::initializer_list<base::StackType>{ 7 });
	test_dataStack("1 + if ( 1 + 4 == 22 + 33 ) { 2 } else { 3 } + 4", std::initializer_list<base::StackType>{ 8 });

	// else if
	test_dataStack("if ( false ) { 1 } else if ( false ) { 2 } else if (false) {3} else { 4 }", std::initializer_list<base::StackType>{4});

	test_dataStack("if ( false ) { 1 } else if ( false ) { 2 } else if (true) {3} else { 4 }", std::initializer_list<base::StackType>{3});
	test_dataStack("if ( false ) { 1 } else if ( true ) { 2 } else if (false) {3} else { 4 }", std::initializer_list<base::StackType>{2});
	test_dataStack("if ( true ) { 1 } else if ( false ) { 2 } else if (false) {3} else { 4 }", std::initializer_list<base::StackType>{1});

	test_dataStack("if ( false ) { 1 } else if ( true ) { 2 } else if (true) {3} else { 4 }", std::initializer_list<base::StackType>{2});
	test_dataStack("if ( true ) { 1 } else if ( true ) { 2 } else if (false) {3} else { 4 }", std::initializer_list<base::StackType>{1});
	test_dataStack("if ( true ) { 1 } else if ( false ) { 2 } else if (true) {3} else { 4 }", std::initializer_list<base::StackType>{1});

	test_dataStack("if ( true ) { 1 } else if ( true ) { 2 } else if (true) {3} else { 4 }", std::initializer_list<base::StackType>{1});
}