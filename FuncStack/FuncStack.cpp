﻿// FuncStack.cpp : Defines the entry point for the application.
//

#define CATCH_CONFIG_RUNNER
#define CATCH_CONFIG_CONSOLE_WIDTH 200

#include "test/OperatorTest.h"
#include "test/ParserTest.h"
#include "test/VariableTest.h"
#include "test/ControlFlowTest.h"

#include "test/catch.hpp"

#include "src/Parser/Parser.h"

template <typename T>
void printSize(const char* name) {
	std::cout << "Sizeof " << name << ":\t" << sizeof(T) << " Bytes\t" << (sizeof(T) / 8) << " ints" << std::endl;
}

int main(int argc, char* argv[]) {
	Catch::Session session;
	session.configData().showSuccessfulTests = false;
	session.configData().showDurations = Catch::ShowDurations::Always;
	const int testReturn = session.run(argc, argv);

	printSize<base::Operation>("Operation");
	printSize<base::StackFrame>("StackFrame");
	printSize<base::BasicType>("BasicType");

	return testReturn;
}