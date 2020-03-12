#pragma once

#include "ShuntingYard.h"
#include "Lexer/Tokenizer.h"
#include "Lexer/Grouping.h"

namespace parser {
		std::list<base::OpCode> parse(std::string&& expression) {
			std::vector<base::OpCode> tokens = lexer::tokenize(expression);
			std::vector<lexer::Node> nodes = lexer::GroupOrganizer::run(tokens);
			std::list<base::OpCode> ops = ShuntingYard::run(nodes);

			std::reverse(ops.begin(), ops.end());

			return ops;
		}
} // namespace parser