#pragma once

#include "ShuntingYard.h"
#include "Tokenizer.h"
#include "PostParser.h"
#include "src/Base/Program.h"
#include "src/Base/Source.h"

namespace compiler {
	class Compiler {
		const base::Source source;
		base::Program program;
		const bool success = true;


		void assume(bool condition, const std::string& message) const {
			if (!condition) {
				throw ex::Exception(message);
			}
		}

	public:
		Compiler(std::string&& expression) : source(std::move(expression)) {
			Tokenizer tokenizer(source);
			const std::vector<Token> tokens = tokenizer.run();
			assume(tokenizer.isSuccess(), "Errors during tokenizing");

			ShuntingYard shunting(tokens, source);
			program = shunting.run();
			assume(shunting.isSuccess(), "Errors during compilation");

			//PostParser postParser(program);
			//postParser.run();
		}

		const base::Program& getProgram() const {
			return program;
		}

		bool isSuccessful() const {
			return success;
		}
	};
}