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

namespace parser {
	class ShuntingYard {
		using Iterator = std::vector<lexer::Node>::const_iterator;

		const Source& source;
		const Iterator end;
		Iterator current;

		std::list<base::Operation> program;
		bool success = true;

		void assume(bool condition, const std::string& message, size_t pos) const {
			if (!condition) {
				throw ex::ParserException(message, pos);
			}
		}

		void assume_isOperator(OpCode is, OpCode should, size_t pos) const {
			assume(is == should, "Node is " + opCodeName(is) + ", should be: " + opCodeName(should), pos);
		}

		size_t nextIndex() const {
			static size_t index = 0;
			return index++;
		}

		void runToNextSync() {
			while ((current != end) and (utils::any_of(current->getToken().id, { OpCode::END_STATEMENT, OpCode::BRACKET_ROUND_CLOSE, OpCode::BRACKET_CURLY_CLOSE }))) {
				current++;
			}
		}

		int getCheckedPriority(const Token& token) const {
			const int priority = token.prio();
			assume(priority != -1, "OpCode has no priority: " + opCodeName(token.id), token.pos);
			return priority;
		}

		BasicType literalToValue(const Token::Type& t) const {
			if (std::holds_alternative<sm_int>(t)) {
				return BasicType(std::get<sm_int>(t));
			} else if (std::holds_alternative<sm_uint>(t)) {
				return BasicType(std::get<sm_uint>(t));
			} else if (std::holds_alternative<sm_float>(t)) {
				return BasicType(std::get<sm_float>(t));
			} else if (std::holds_alternative<sm_bool>(t)) {
				return BasicType(std::get<sm_bool>(t));
			}
		}

		std::list<Operation> shuntingYard(const std::vector<lexer::Node>& vec) const {
			return shuntingYard(vec.begin(), vec.end());
		}

		std::list<Operation> shuntingYard(Iterator this_current, Iterator this_end) const {
			std::stack<lexer::Node, std::vector<lexer::Node>> sideStack;
			std::vector<lexer::Node> sortedTokens;

			for (; this_current != this_end; this_current++) {
				if (this_current->is(OpCode::LITERAL) or this_current->is(OpCode::NAME) or this_current->isGroup()) {
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
				const OpCode opCode = it->getOpCode();
				switch (opCode) {
					case OpCode::ADD: // fallthrough
					case OpCode::SUB: // fallthrough
					case OpCode::MULT: // fallthrough
					case OpCode::DIV: // fallthrough
					case OpCode::INCR: // fallthrough
					case OpCode::DECR: // fallthrough
					case OpCode::EQ: // fallthrough
					case OpCode::UNEQ: // fallthrough
					case OpCode::BIGGER: // fallthrough
					case OpCode::LESS:
						program.push_back(base::Operation(opCode));
						break;
					case OpCode::LITERAL:
						program.push_back(base::Operation(OpCode::LOAD, StackFrame(BasicType(literalToValue(it->getToken().value)))));
						break;
					case OpCode::NAME:
						program.push_back(base::Operation(OpCode::LOAD, StackFrame(std::get<std::string>(it->getToken().value))));
						break;
					case OpCode::BRACKET_ROUND_OPEN:
						program.splice(program.end(), shuntingYard(it->getGroup()));
				}
			}

			return program;
		}

		void insertParsed(const lexer::Node& source) {
			std::list<base::Operation> ops = shuntingYard(source.getGroup());
			program.splice(program.end(), std::move(ops));
		}

		void parseSubGroup(OpCode expectedOperator) {
			const lexer::Node next = *current++;
			assume_isOperator(next.getOpCode(), expectedOperator, next.getToken().pos);
			insertParsed(next);
		}

		void insert_if_base(size_t jumpToEndIndex) {
			parseSubGroup(OpCode::BRACKET_ROUND_OPEN);

			const size_t jumpOverIndex = nextIndex();
			program.emplace_back(OpCode::JUMP_LABEL_IF, base::StackFrame(base::BasicType(jumpOverIndex))); // jump over if

			parseSubGroup(OpCode::BRACKET_CURLY_OPEN);

			program.emplace_back(OpCode::JUMP_LABEL, base::StackFrame(base::BasicType(jumpToEndIndex))); // jump to end
			program.emplace_back(OpCode::LABEL, base::StackFrame(base::BasicType(jumpOverIndex))); // after if label
		}

		void parse_if() {
			current++; // "if"
			const size_t jumpToEndIndex = nextIndex();
			insert_if_base(jumpToEndIndex);
			program.emplace_back(OpCode::LABEL, base::StackFrame(base::BasicType(jumpToEndIndex))); // set end label
		}

		void parse_else() {
			current++; // "else"
			Operation jumpToEndLabel = program.back(); // retrieve 'else' end label
			assume_isOperator(jumpToEndLabel.getOpCode(), OpCode::LABEL, 0);
			program.pop_back();

			if (current->is(OpCode::IF)) {
				current++;

				const size_t jumpToEndIndex = jumpToEndLabel.firstValue().getValue().getUint();
				insert_if_base(jumpToEndIndex);
			} else {
				parseSubGroup(OpCode::BRACKET_CURLY_OPEN);
			}

			program.push_back(std::move(jumpToEndLabel)); // put back 'else' end label
		}

		void embeddCodeStatement() {
			const auto endStatementIt = std::find_if(current, end, [](const lexer::Node& n) {
				return n.getOpCode() == OpCode::END_STATEMENT;
			});
			assume(endStatementIt != end, "Missing end statement!", current->getToken().pos);

			program.splice(program.end(), shuntingYard(current, endStatementIt));
			current = endStatementIt + 1;
		}

	public:
		bool isSuccess() const {
			return success;
		}

		ShuntingYard(Iterator begin, Iterator end, const Source& source)
			: current(begin), end(end), source(source) {
		}

		ShuntingYard(const std::vector<lexer::Node>& nodes , const Source& source)
			: current(nodes.begin()), end(nodes.end()), source(source) {
		}

		std::list<base::Operation> run() {
			using namespace base;

			try {
				while (current != end) {
					switch (current->getOpCode()) {
						case OpCode::TYPE:
						{
							StackFrame variableType(literalToValue(current->getToken().value));
							current++;
							StackFrame variableName(std::get<std::string>(current->getToken().value));
							program.push_back(Operation(OpCode::CREATE, std::move(variableType), std::move(variableName)));
						}
						break;
						case OpCode::NAME:
							if ((current + 1)->is(OpCode::ASSIGN)) {
								std::string variableName = std::get<std::string>(current->getToken().value);
								current += 2;
								embeddCodeStatement();
								program.push_back(Operation(OpCode::STORE, StackFrame(std::move(variableName))));
							} else {
								embeddCodeStatement();
							}
							break;
						case OpCode::IF:
							parse_if();
							break;
						case OpCode::ELSE:
							parse_else();
							break;
						case OpCode::BRACKET_ROUND_OPEN:
							insertParsed(*current);
							break;
						case OpCode::BRACKET_CURLY_OPEN:
							program.push_back(Operation(OpCode::BRACKET_CURLY_OPEN));
							insertParsed(*current);
							program.push_back(Operation(OpCode::BRACKET_CURLY_CLOSE));
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