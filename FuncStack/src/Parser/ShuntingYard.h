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
		std::stack<base::Operation> sideStack;
		std::list<base::Operation> opStack;
		const std::vector<lexer::Node>& _tokens;
		std::vector<lexer::Node>::const_iterator _currentPos;

		void assure_isOperator(const lexer::Node& node, base::Operator op) const {
			ex::assure(node.is(op), "Node is "s + base::getName(node.getOperator()) + ", should be: " + base::getName(op));
		}

		void assure_isOperator(const base::Operation& opCode, base::Operator op) const {
			ex::assure(opCode.getOperator() == op, "Node is "s + base::getName(opCode.getOperator()) + ", should be: " + base::getName(op));
		}

		size_t nextIndex() const {
			static size_t index = 0;
			return index++;
		}

		void insertParsed(lexer::Node&& source) {
			const std::list<base::Operation> ops = ShuntingYard::run(source.getGroup());
			opStack.insert(opStack.end(), ops.begin(), ops.end());
		}

		void parseSubGroup(base::Operator expectedOperator) {
			lexer::Node next = *_currentPos++;
			assure_isOperator(next, expectedOperator);
			insertParsed(std::move(next));
		}

		void insert_if_base(size_t jumpToEndIndex) {
			parseSubGroup(Operator::BRACE_GROUP);

			const size_t jumpOverIndex = nextIndex();
			opStack.emplace_back(Operator::JUMP_LABEL_IF, base::StackFrame(base::BasicType(jumpOverIndex))); // jump over if

			parseSubGroup(Operator::BRACKET_GROUP);

			opStack.emplace_back(Operator::JUMP_LABEL, base::StackFrame(base::BasicType(jumpToEndIndex))); // jump to end
			opStack.emplace_back(Operator::LABEL, base::StackFrame(base::BasicType(jumpOverIndex))); // after if label
		}

		void parse_if() {
			const size_t jumpToEndIndex = nextIndex();
			insert_if_base(jumpToEndIndex);
			opStack.emplace_back(Operator::LABEL, base::StackFrame(base::BasicType(jumpToEndIndex))); // set end label
		}

		void parse_else() {
			Operation jumpToEndLabel = opStack.back(); // retrieve 'else' end label
			assure_isOperator(jumpToEndLabel, Operator::LABEL);
			opStack.pop_back();

			if (_currentPos->is(Operator::IF)) {
				_currentPos++;

				const size_t jumpToEndIndex = jumpToEndLabel.getStackFrame(0).getValue().getUint();
				insert_if_base(jumpToEndIndex);
			} else {
				parseSubGroup(Operator::BRACKET_GROUP);
			}

			opStack.push_back(std::move(jumpToEndLabel)); // put back 'else' end label
		}

		void shuntingYardIteration(lexer::Node&& op) {
			while (!sideStack.empty() and (sideStack.top().priority() >= op.getOperation().priority())) {
				opStack.push_back(sideStack.top());
				sideStack.pop();
			}
			sideStack.push(op.getOperation());
		}

		void flushSideStack() {
			while (!sideStack.empty()) {
				opStack.push_back(sideStack.top());
				sideStack.pop();
			}
		}

		ShuntingYard(const std::vector<lexer::Node>& tokens) : _tokens(tokens) {
			using namespace base;

			const auto equalIt = std::find_if(tokens.begin(), tokens.end(), [](const lexer::Node& n) {
				return n.getOperator() == Operator::STORE;
			});

			while (_currentPos != _tokens.end()) {
				auto op = *_currentPos++;
				switch (op.getOperator()) {
					case Operator::LOAD:
						opStack.push_back(op.getOperation());
						break;
					case Operator::IF:
						parse_if();
						break;
					case Operator::ELSE:
						parse_else();
						break;
					case Operator::BRACE_GROUP:
						insertParsed(std::move(op));
						break;
					case Operator::BRACKET_GROUP:
						opStack.push_back(Operation(Operator::BRACKET_OPEN));
						insertParsed(std::move(op));
						opStack.push_back(Operation(Operator::BRACKET_CLOSE));
						break;
					default: shuntingYardIteration(std::move(op));
				}
			}

			flushSideStack();
		}

	public:
		static std::list<base::Operation> run(const std::vector<lexer::Node>& tokens) {
			return std::move(ShuntingYard(tokens).opStack);
		}
	};
} // namespace parser