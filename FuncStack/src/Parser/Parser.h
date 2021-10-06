#pragma once

#include "ShuntingYard.h"
#include "Lexer/Tokenizer.h"
#include "Lexer/Grouping.h"
#include "PostParser.h"

namespace parser {
		std::list<base::Operation> parse(std::string&& expression) {
			std::vector<base::Operation> tokens = lexer::Tokenizer::run(expression);
			std::vector<lexer::Node> nodes = lexer::GroupOrganizer::run(tokens);
			std::list<base::Operation> ops = ShuntingYard::run(nodes);
			PostParser::run(ops);
			return ops;
		}
} // namespace parser