// FuncStack.cpp : Defines the entry point for the application.
//

#define CATCH_CONFIG_RUNNER
#define CATCH_CONFIG_CONSOLE_WIDTH 200

#include "test/OperatorTest.h"
#include "test/ParserTest.h"
#include "test/VariableTest.h"
#include "test/ControlFlowTest.h"

#include "test/catch.hpp"

#include "src/Parser/Parser.h"

int main(int argc, char* argv[]) {
	auto session = Catch::Session();
	session.configData().showSuccessfulTests = false;
	session.configData().showDurations = Catch::ShowDurations::Always;
	return session.run(argc, argv);
}