#pragma once

#include "ShuntingYard.h"
#include "Lexer/Tokenizer.h"
#include "Lexer/Grouping.h"
#include "PostParser.h"

namespace parser {
		std::list<base::StackFrame> parse(std::string&& expression) {
			std::vector<base::StackFrame> tokens = lexer::tokenize(expression);
			std::vector<lexer::Node> nodes = lexer::GroupOrganizer::run(tokens);
			std::list<base::StackFrame> ops = ShuntingYard::run(nodes);
			PostParser::run(ops);
			return ops;
		}
} // namespace parser