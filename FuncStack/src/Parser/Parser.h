#pragma once

#include "ShuntingYard.h"
#include "Lexer/Tokenizer.h"
#include "Lexer/Grouping.h"
#include "PostParser.h"
#include "src/Base/Source.h"

class Compiler {
	const Source source;
	std::list<base::Operation> program;
	const bool success = true;


	void assume(bool condition, const std::string& message) const {
		if (!condition) {
			throw ex::Exception(message);
		}
	}

public:
	Compiler(std::string&& expression) : source(std::move(expression)) {
		lexer::Tokenizer tokenizer(source);
		const std::vector<Token> tokens = tokenizer.run();
		assume(tokenizer.isSuccess(), "Errors during tokenizing");

		lexer::GroupOrganizer grouping(tokens, source);
		const std::vector<lexer::Node> nodes = grouping.run();
		assume(grouping.isSuccess(), "Errors during grouping");

		parser::ShuntingYard shunting(nodes, source);
		program = shunting.run();
		assume(shunting.isSuccess(), "Errors during shunting yard");
		
		parser::PostParser::run(program);
	}

	const std::list<base::Operation>& getProgram() const {
		return program;
	}

	bool isSuccessful() const {
		return success;
	}
};