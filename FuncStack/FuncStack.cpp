// FuncStack.cpp : Defines the entry point for the application.
//

#define CATCH_CONFIG_RUNNER
#define CATCH_CONFIG_CONSOLE_WIDTH 200

#include "test/TokenizerTest.h"
#include "test/OperatorTest.h"
#include "test/ParserTest.h"
#include "test/VariableTest.h"
#include "test/ControlFlowTest.h"
#include "test/FunctionTest.h"
#include "test/CompleteTest.h"

#include "test/catch.hpp"

#include "test/Benchmarks/Tokenizer_Numbers.h"

/* TODO
	- String interning
*/

template <typename T>
void printSize(const char* name) {
	std::cout << "Sizeof " << name << ": " << std::setw(2) << std::left << sizeof(T) << " Bytes (" << (sizeof(T) / 8) << " ints)" << std::endl;
}

int main(int argc, char* argv[]) {
	Catch::Session session;
	session.configData().showSuccessfulTests = false;
	session.configData().showDurations = Catch::ShowDurations::Always;
	const int testReturn = session.run(argc, argv);

	benchmark::tokenizer::run();

	printSize<base::Operation>("Operation");
	printSize<base::BasicType>("BasicType");
	std::cout << "Number of OpCodes: " << static_cast<int>(base::OpCode::END_ENUM_OPCODE) << std::endl;

	return testReturn;
}