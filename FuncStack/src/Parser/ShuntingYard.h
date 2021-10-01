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
		std::stack<base::StackFrame> sideStack;
		std::list<base::StackFrame> opStack;
		utils::Stream<lexer::Node>& opStream;

		void assure_isOperator(const lexer::Node& node, base::Operator op) const {
			ex::assure(node.is(op), "Node is "s + base::getName(node.getOperator()) + ", should be: " + base::getName(op));
		}

		void assure_isOperator(const base::StackFrame& opCode, base::Operator op) const {
			ex::assure(opCode.getOperator() == op, "Node is "s + base::getName(opCode.getOperator()) + ", should be: " + base::getName(op));
		}

		size_t nextIndex() const {
			static size_t index = 0;
			return index++;
		}

		void insertParsed(lexer::Node&& source) {
			const std::list<base::StackFrame> ops = ShuntingYard::run(source.getGroup());
			opStack.insert(opStack.end(), ops.begin(), ops.end());
		}

		void parseSubGroup(base::Operator expectedOperator) {
			lexer::Node next = opStream.peakAndNext();
			assure_isOperator(next, expectedOperator);
			insertParsed(std::move(next));
		}

		void insert_if_base(size_t jumpToEndIndex) {
			parseSubGroup(Operator::BRACE_GROUP);

			const size_t jumpOverIndex = nextIndex();
			opStack.emplace_back(Operator::JUMP_LABEL_IF, base::StackType(base::ValueType(jumpOverIndex))); // jump over if

			parseSubGroup(Operator::BRACKET_GROUP);

			opStack.emplace_back(Operator::JUMP_LABEL, base::StackType(base::ValueType(jumpToEndIndex))); // jump to end
			opStack.emplace_back(Operator::LABEL, base::StackType(base::ValueType(jumpOverIndex))); // after if label
		}

		void parse_if() {
			const size_t jumpToEndIndex = nextIndex();
			insert_if_base(jumpToEndIndex);
			opStack.emplace_back(Operator::LABEL, base::StackType(base::ValueType(jumpToEndIndex))); // set end label
		}

		void parse_else() {
			StackFrame jumpToEndLabel = opStack.back(); // retrieve 'else' end label
			assure_isOperator(jumpToEndLabel, Operator::LABEL);
			opStack.pop_back();

			if (opStream->is(Operator::IF)) {
				opStream.next();

				const int jumpToEndIndex = jumpToEndLabel.value().getValue().getUnsigned();
				insert_if_base(jumpToEndIndex);
			}
			else {
				parseSubGroup(Operator::BRACKET_GROUP);
			}

			opStack.push_back(std::move(jumpToEndLabel)); // put back 'else' end label
		}

		void shuntingYardIteration(lexer::Node&& op) {
			while (!sideStack.empty() and (sideStack.top().priority() >= op.getStackFrame().priority())) {
				opStack.push_back(sideStack.top());
				sideStack.pop();
			}
			sideStack.push(op.getStackFrame());
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
					case Operator::LOAD: opStack.push_back(op.getStackFrame()); break;
					case Operator::IF: parse_if(); break;
					case Operator::ELSE: parse_else(); break;
					case Operator::BRACE_GROUP: insertParsed(std::move(op)); break;
					case Operator::BRACKET_GROUP: insertParsed(std::move(op)); break;
					default: shuntingYardIteration(std::move(op));
				}
			}

			flushSideStack();
		}

	public:
		static std::list<base::StackFrame> run(utils::Stream<lexer::Node> opStream) {
			return ShuntingYard(opStream).opStack;
		}
	};
} // namespace parser