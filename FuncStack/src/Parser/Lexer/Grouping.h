#pragma once

#include "Node.h"
#include "src/Utils/Stream.h"

#include <memory>

namespace lexer {
	class GroupOrganizer {
		using it = std::vector<base::OpCode>::const_iterator;
		using OpStream = utils::Stream<base::OpCode>;

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
			for (const auto braceGroup : { base::Operator::BRACE_GROUP, base::Operator::BRACKET_GROUP }) {
				const auto [open, close] = FromGroupBrace(braceGroup);

				if (inputTokens->isOperator(open)) {
					inputTokens.next();
					const auto _begin = inputTokens.pos();
					unwind(open, close);

					outputNodes.emplace_back(braceGroup, GroupOrganizer::run(OpStream(_begin, inputTokens.pos())));
					return true;
				}
			}

			return false;
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