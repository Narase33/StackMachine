#pragma once

#include "Node.h"
#include "src/Utils/Stream.h"

#include <memory>

namespace lexer {
	class GroupOrganizer {
		using it = std::vector<base::StackFrame>::const_iterator;
		using OpStream = utils::Stream<base::StackFrame>;

		OpStream inputTokens;
		std::vector<Node> outputNodes;

		void unwind(base::Operator open, base::Operator close) {
			int openGroups = 1;
			while ((openGroups > 0) and !inputTokens.isEnd()) {
				if (inputTokens->isOperator(open))
					openGroups++;

				if (inputTokens->isOperator(close))
					openGroups--;

				inputTokens.next();
			}
			inputTokens.prev();
		}

		bool extractGroup() {
			base::Operator open = base::Operator::NONE;
			base::Operator close = base::Operator::NONE;
			base::Operator group = base::Operator::NONE;

			if (inputTokens->isOperator(base::Operator::BRACE_OPEN)) {
				std::tie(open, close) = FromGroupBrace(base::Operator::BRACE_GROUP);
				group = base::Operator::BRACE_GROUP;
			}
			else if (inputTokens->isOperator(base::Operator::BRACKET_OPEN)) {
				std::tie(open, close) = FromGroupBrace(base::Operator::BRACKET_GROUP);
				group = base::Operator::BRACKET_GROUP;
			}
			else {
				return false;
			}

			inputTokens.next();
			const auto _begin = inputTokens.pos();
			unwind(open, close);

			outputNodes.emplace_back(base::StackFrame(group), GroupOrganizer::run(OpStream(_begin, inputTokens.pos())));
			return true;
		}

		GroupOrganizer(OpStream tokenStream) : inputTokens(tokenStream) {
			while (!inputTokens.isEnd()) {
				const bool success = extractGroup();
				if (!success) {
					outputNodes.emplace_back(inputTokens.peak());
				}
				inputTokens.next();
			}
		}

	public:
		static std::vector<Node> run(OpStream tokenStream) {
			return GroupOrganizer(tokenStream).outputNodes;
		}
	};
}