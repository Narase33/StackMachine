#pragma once

#include "Node.h"
#include "src/Utils/Stream.h"

#include <memory>

namespace lexer {
	/*
	Class convertes code between braces into recursive nodes representing those braces
	*/
	class GroupOrganizer {
		using Iterator = std::vector<Token>::const_iterator;

		const Source& source;
		Iterator current;
		const Iterator end;
		const Iterator begin;

		std::vector<Node> outputNodes;
		bool success = true;

		void assume(bool condition, const std::string& message, size_t pos) const {
			if (!condition) {
				throw ex::ParserException(message, pos);
			}
		}

		void jumpToOpposingBracket(OpCode open, OpCode close) {
			const Iterator origin = current;

			int openGroups = 1;
			while (openGroups > 0) {
				assume(current != end, "Missing closing bracket", origin->pos);

				if (current->id == open)
					openGroups++;

				if (current->id == close)
					openGroups--;

				current++;
			}
			current--;
		}

		bool extractGroup() {
			OpCode open = OpCode::ERR;
			OpCode close = OpCode::ERR;

			if (current->id == OpCode::BRACKET_ROUND_OPEN) {
				open = OpCode::BRACKET_ROUND_OPEN;
				close = OpCode::BRACKET_ROUND_CLOSE;
			} else if (current->id == OpCode::BRACKET_CURLY_OPEN) {
				open = OpCode::BRACKET_CURLY_OPEN;
				close = OpCode::BRACKET_CURLY_CLOSE;
			} else {
				return false;
			}

			Token groupToken = *current;

			current++;
			const Iterator begin = current;
			jumpToOpposingBracket(open, close);

			GroupOrganizer organizer(begin, current, source);
			outputNodes.push_back(lexer::Node(std::move(groupToken), organizer.run()));
			return true;
		}

	public:
		bool isSuccess() const {
			return success;
		}
		
		GroupOrganizer(Iterator begin, Iterator end, const Source& source) :
			begin(begin), current(begin), end(end), source(source) {
		}

		GroupOrganizer(const std::vector<Token>& nodes, const Source& source) :
			begin(nodes.begin()), current(nodes.begin()), end(nodes.end()), source(source) {
		}

		std::vector<Node> run() {
			try {
				while (current != end) {
					const bool success = extractGroup();
					if (!success) {
						outputNodes.push_back(lexer::Node(*current));
					}
					current++;
				}
			} catch (const ex::ParserException& ex) {
				std::cout << ex.what() << "\n" << source.markedLineAt(ex.getPos()) << std::endl;
			}

			return std::move(outputNodes);
		}
	};
}