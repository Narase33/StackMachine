#pragma once

#include "ShuntingYard.h"
#include "Lexer/Tokenizer.h"
#include "Lexer/Grouping.h"
#include "PostParser.h"
#include "src/Base/Source.h"

namespace compiler {
	class Compiler {
		const base::Source source;
		std::list<base::Operation> program;
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

			GroupOrganizer grouping(tokens, source);
			const std::vector<Node> nodes = grouping.run();
			assume(grouping.isSuccess(), "Errors during grouping");

			ShuntingYard shunting(nodes, source);
			program = shunting.run();
			assume(shunting.isSuccess(), "Errors during shunting yard");

			PostParser::run(program);
		}

		const std::list<base::Operation>& getProgram() const {
			return program;
		}

		bool isSuccessful() const {
			return success;
		}
	};
}