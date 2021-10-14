#pragma once

#include <algorithm>
#include <sstream>
#include <stack>
#include <list>
#include <vector>

#include "Lexer/Node.h"

#include "src/Utils/Stream.h"
#include "src/Exception.h"

#include "src/Base/Source.h"

namespace compiler {
	class ShuntingYard {
		using Iterator = std::vector<Node>::const_iterator;

		const base::Source& source;
		const Iterator end;
		Iterator current;

		std::list<base::Operation> program;
		bool success = true;

		void assume(bool condition, const std::string& message, size_t pos) const {
			if (!condition) {
				throw ex::ParserException(message, pos);
			}
		}

		void assume_isOperator(base::OpCode is, base::OpCode should, size_t pos) const {
			assume(is == should, "Node is " + opCodeName(is) + ", should be: " + opCodeName(should), pos);
		}

		size_t nextIndex() const {
			static size_t index = 0;
			return index++;
		}

		void runToNextSync() {
			while ((current != end) and (utils::any_of(current->getToken().id, { base::OpCode::END_STATEMENT, base::OpCode::BRACKET_ROUND_CLOSE, base::OpCode::BRACKET_CURLY_CLOSE }))) {
				current++;
			}
		}

		int getCheckedPriority(const Token& token) const {
			const int priority = token.prio();
			assume(priority != -1, "OpCode has no priority: " + opCodeName(token.id), token.pos);
			return priority;
		}

		base::BasicType literalToValue(const Token::Type& t) const {
			if (std::holds_alternative<base::sm_int>(t)) {
				return base::BasicType(std::get<base::sm_int>(t));
			} else if (std::holds_alternative<base::sm_uint>(t)) {
				return base::BasicType(std::get<base::sm_uint>(t));
			} else if (std::holds_alternative<base::sm_float>(t)) {
				return base::BasicType(std::get<base::sm_float>(t));
			} else if (std::holds_alternative<base::sm_bool>(t)) {
				return base::BasicType(std::get<base::sm_bool>(t));
			}
		}

		std::list<base::Operation> shuntingYard(const std::vector<Node>& vec) const {
			return shuntingYard(vec.begin(), vec.end());
		}

		std::list<base::Operation> shuntingYard(Iterator this_current, Iterator this_end) const {
			std::stack<Node, std::vector<Node>> sideStack;
			std::vector<Node> sortedTokens;

			for (; this_current != this_end; this_current++) {
				if (this_current->is(base::OpCode::LITERAL) or this_current->is(base::OpCode::NAME) or this_current->isGroup()) {
					sortedTokens.push_back(*this_current);
					continue;
				}

				while (!sideStack.empty() and (getCheckedPriority(sideStack.top().getToken()) >= getCheckedPriority(this_current->getToken()))) {
					sortedTokens.push_back(sideStack.top());
					sideStack.pop();
				}
				sideStack.push(this_current->getToken());
			}

			while (!sideStack.empty()) {
				sortedTokens.push_back(sideStack.top());
				sideStack.pop();
			}

			std::list<base::Operation> program;
			for (auto it = sortedTokens.begin(); it != sortedTokens.end(); it++) {
				const base::OpCode opCode = it->getOpCode();
				switch (opCode) {
					case base::OpCode::ADD: // fallthrough
					case base::OpCode::SUB: // fallthrough
					case base::OpCode::MULT: // fallthrough
					case base::OpCode::DIV: // fallthrough
					case base::OpCode::INCR: // fallthrough
					case base::OpCode::DECR: // fallthrough
					case base::OpCode::EQ: // fallthrough
					case base::OpCode::UNEQ: // fallthrough
					case base::OpCode::BIGGER: // fallthrough
					case base::OpCode::LESS:
						program.push_back(base::Operation(opCode));
						break;
					case base::OpCode::LITERAL:
						program.push_back(base::Operation(base::OpCode::LOAD, base::StackFrame(base::BasicType(literalToValue(it->getToken().value)))));
						break;
					case base::OpCode::NAME:
						program.push_back(base::Operation(base::OpCode::LOAD, base::StackFrame(std::get<std::string>(it->getToken().value))));
						break;
					case base::OpCode::BRACKET_ROUND_OPEN:
						program.splice(program.end(), shuntingYard(it->getGroup()));
				}
			}

			return program;
		}

		void insertParsed(const Node& source) {
			std::list<base::Operation> ops = shuntingYard(source.getGroup());
			program.splice(program.end(), std::move(ops));
		}

		void parseSubGroup(base::OpCode expectedOperator) {
			const Node next = *current++;
			assume_isOperator(next.getOpCode(), expectedOperator, next.getToken().pos);
			insertParsed(next);
		}

		void insert_if_base(size_t jumpToEndIndex) {
			parseSubGroup(base::OpCode::BRACKET_ROUND_OPEN);

			const size_t jumpOverIndex = nextIndex();
			program.emplace_back(base::OpCode::JUMP_LABEL_IF, base::StackFrame(base::BasicType(jumpOverIndex))); // jump over if

			parseSubGroup(base::OpCode::BRACKET_CURLY_OPEN);

			program.emplace_back(base::OpCode::JUMP_LABEL, base::StackFrame(base::BasicType(jumpToEndIndex))); // jump to end
			program.emplace_back(base::OpCode::LABEL, base::StackFrame(base::BasicType(jumpOverIndex))); // after if label
		}

		void parse_if() {
			current++; // "if"
			const size_t jumpToEndIndex = nextIndex();
			insert_if_base(jumpToEndIndex);
			program.emplace_back(base::OpCode::LABEL, base::StackFrame(base::BasicType(jumpToEndIndex))); // set end label
		}

		void parse_else() {
			current++; // "else"
			base::Operation jumpToEndLabel = program.back(); // retrieve 'else' end label
			assume_isOperator(jumpToEndLabel.getOpCode(), base::OpCode::LABEL, 0);
			program.pop_back();

			if (current->is(base::OpCode::IF)) {
				current++;

				const size_t jumpToEndIndex = jumpToEndLabel.firstValue().getValue().getUint();
				insert_if_base(jumpToEndIndex);
			} else {
				parseSubGroup(base::OpCode::BRACKET_CURLY_OPEN);
			}

			program.push_back(std::move(jumpToEndLabel)); // put back 'else' end label
		}

		void embeddCodeStatement() {
			const auto endStatementIt = std::find_if(current, end, [](const Node& n) {
				return n.getOpCode() == base::OpCode::END_STATEMENT;
			});
			assume(endStatementIt != end, "Missing end statement!", current->getToken().pos);

			program.splice(program.end(), shuntingYard(current, endStatementIt));
			current = endStatementIt + 1;
		}

	public:
		bool isSuccess() const {
			return success;
		}

		ShuntingYard(Iterator begin, Iterator end, const base::Source& source)
			: current(begin), end(end), source(source) {
		}

		ShuntingYard(const std::vector<Node>& nodes , const base::Source& source)
			: current(nodes.begin()), end(nodes.end()), source(source) {
		}

		std::list<base::Operation> run() {
			try {
				while (current != end) {
					switch (current->getOpCode()) {
						case base::OpCode::TYPE:
						{
							base::StackFrame variableType(literalToValue(current->getToken().value));
							current++;
							base::StackFrame variableName(std::get<std::string>(current->getToken().value));
							program.push_back(base::Operation(base::OpCode::CREATE, std::move(variableType), std::move(variableName)));
						}
						break;
						case base::OpCode::NAME:
							if ((current + 1)->is(base::OpCode::ASSIGN)) {
								std::string variableName = std::get<std::string>(current->getToken().value);
								current += 2;
								embeddCodeStatement();
								program.push_back(base::Operation(base::OpCode::STORE, base::StackFrame(std::move(variableName))));
							} else {
								embeddCodeStatement();
							}
							break;
						case base::OpCode::IF:
							parse_if();
							break;
						case base::OpCode::ELSE:
							parse_else();
							break;
						case base::OpCode::BRACKET_ROUND_OPEN:
							insertParsed(*current);
							break;
						case base::OpCode::BRACKET_CURLY_OPEN:
							program.push_back(base::Operation(base::OpCode::BRACKET_CURLY_OPEN));
							insertParsed(*current);
							program.push_back(base::Operation(base::OpCode::BRACKET_CURLY_CLOSE));
							break;
						default:
							embeddCodeStatement();
					}
				}
			} catch (const ex::ParserException& ex) {
				std::cout << ex.what() << "\n" << source.markedLineAt(ex.getPos()) << std::endl;
				runToNextSync();
				success = false;
			}

			return std::move(program);
		}
	};
} // namespace parser