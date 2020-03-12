#pragma once

#include <algorithm>
#include <sstream>
#include <stack>
#include <list>
#include <vector>

#include "Lexer/Node.h"

#include "src/Utils/Stream.h"
#include "src/Exception.h"

namespace parser {
	class ShuntingYard {
		std::stack<base::OpCode> sideStack;
		std::list<base::OpCode> opStack;
		utils::Stream<lexer::Node>& opStream;

		void assure_isOperator(const lexer::Node& node, base::Operator op) const {
			ex::assure(node.is(op), "Node is "s + base::getName(node.getOperator()) + ", should be: " + base::getName(op));
		}

		int nextIndex() const {
			static int index = 0;
			return index++;
		}

		void insertParsed(lexer::Node&& source) {
			const std::list<base::OpCode> ops = ShuntingYard::run(source.getGroup());
			opStack.insert(opStack.end(), ops.begin(), ops.end());
		}

		void slideExpected(base::Operator expectedOperator) {
			lexer::Node next = opStream.peakAndNext();
			assure_isOperator(next, expectedOperator);
			insertParsed(std::move(next));
		}

		void insert_if_base(int jumpToEndIndex) {
			slideExpected(Operator::BRACE_GROUP);

			const int jumpOverIndex = nextIndex();
			opStack.emplace_back(Operator::JUMP_LABEL_IF, jumpOverIndex); // jump over if

			slideExpected(Operator::BRACKET_GROUP);

			opStack.emplace_back(Operator::JUMP_LABEL, jumpToEndIndex); // jump to end
			opStack.emplace_back(Operator::LABEL, jumpOverIndex); // after if label
		}

		void parse_if() {
			const int jumpToEndIndex = nextIndex();
			insert_if_base(jumpToEndIndex);
			opStack.emplace_back(Operator::LABEL, jumpToEndIndex); // set end label
		}

		void parse_else() {
			const OpCode jumpToEndLabel = opStack.back(); // retrieve end label
			assure_isOperator(jumpToEndLabel, Operator::LABEL);
			opStack.pop_back();

			if (opStream->is(Operator::IF)) {
				opStream.next();

				int jumpToEndIndex = jumpToEndLabel.value().get<ValueType>().get<long>();
				insert_if_base(jumpToEndIndex);
			} else {
				slideExpected(Operator::BRACKET_GROUP);
			}

			opStack.push_back(jumpToEndLabel); // put back end label
		}

		void shuntingYardIteration(lexer::Node op) {
			while (!(sideStack.empty() or (sideStack.top() < (op.getOpCode())))) {
				opStack.push_back(sideStack.top());
				sideStack.pop();
			}
			sideStack.push(op.getOpCode());
		}

		void flushSideStack() {
			while (!sideStack.empty()) {
				opStack.push_back(sideStack.top());
				sideStack.pop();
			}
		}

		ShuntingYard(utils::Stream<lexer::Node> opStream) : opStream(opStream) {
			using namespace base;

			while (!opStream.isEnd()) {
				auto op = opStream.peakAndNext();
				switch (op.getOperator()) {
					case Operator::LOAD: opStack.push_back(op.getOpCode()); break;
					case Operator::IF: parse_if(); break;
					case Operator::ELSE: parse_else(); break;
					case Operator::BRACE_GROUP: insertParsed(std::move(op)); break;
					case Operator::BRACKET_GROUP: insertParsed(std::move(op)); break;
					default: shuntingYardIteration(op);
				}
			}

			flushSideStack();
		}

	public:
		static std::list<base::OpCode> run(utils::Stream<lexer::Node> opStream) {
			return ShuntingYard(opStream).opStack;
		}
	};
} // namespace parser