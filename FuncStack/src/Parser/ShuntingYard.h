#pragma once

#include <algorithm>
#include <sstream>
#include <stack>
#include <list>
#include <vector>

#include "Token.h"

#include "src/Utils/Stream.h"
#include "src/Exception.h"

#include "src/Base/Source.h"

namespace compiler {
	class ShuntingYard {
		struct LoopLabels { size_t head, end, scopeLevel; };

		using Iterator = std::vector<Token>::const_iterator;

		const base::Source& source;
		const Iterator _end;
		Iterator _current;

		size_t currentScopeLevel = 0;
		std::stack<LoopLabels, std::vector<LoopLabels>> loopLabels;

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

		void runToNextSync(Iterator& begin, Iterator end) {
			while ((begin != end) and (utils::any_of(begin->id, { base::OpCode::END_STATEMENT, base::OpCode::BRACKET_ROUND_CLOSE, base::OpCode::BRACKET_CURLY_CLOSE }))) {
				begin++;
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

		Iterator unwindGroup(Iterator& begin, Iterator end) {
			const Iterator groupStart = begin;

			auto [bracketBegin, bracketEnd] = base::getBracketGroup(begin->id);
			assume(bracketBegin != base::OpCode::ERR, "Expected group", begin->pos);

			int counter = 1;
			do {
				begin++;
				assume(begin != end, "Group didnt close", groupStart->pos);

				if (begin->id == bracketBegin) counter++;
				if (begin->id == bracketEnd) counter--;
			} while (counter > 0);
			return groupStart;
		}

		std::list<base::Operation> shuntingYard(Iterator begin, Iterator end) const {
			const Iterator this_current_copy = begin;

			std::stack<Token, std::vector<Token>> operatorStack;
			std::vector<Token> sortedTokens;

			for (; begin != end; begin++) {
				if ((begin->id == base::OpCode::LITERAL) or (begin->id == base::OpCode::NAME)) {
					sortedTokens.push_back(*begin);
					continue;
				}

				if (base::isOpeningBracket(begin->id)) {
					operatorStack.push(*begin);
					continue;
				}

				if (base::isClosingBracket(begin->id)) {
					auto [bracketOpen, bracketClose] = base::getBracketGroup(begin->id);

					while (!operatorStack.empty() and (operatorStack.top().id != bracketOpen)) {
						assume(!base::isOpeningBracket(operatorStack.top().id), "Found closing bracket that doesnt match", begin->pos);
						sortedTokens.push_back(operatorStack.top());
						operatorStack.pop();
					}
					assume(operatorStack.size() > 0, "Group didnt end", this_current_copy->pos);

					operatorStack.pop();
					// TODO As soon as there are functions implemented
					// if (operatorStack.top() is function) {
					//		sortedTokens.push_back(operatorStack.top());
					//		operatorStack.pop();
					// }
					continue;
				}

				while (!operatorStack.empty() and (getCheckedPriority(operatorStack.top()) >= getCheckedPriority(*begin))) {
					sortedTokens.push_back(operatorStack.top());
					operatorStack.pop();
				}
				operatorStack.push(*begin);
			}

			while (!operatorStack.empty()) {
				sortedTokens.push_back(operatorStack.top());
				operatorStack.pop();
			}

			std::list<base::Operation> program;
			for (auto it = sortedTokens.begin(); it != sortedTokens.end(); it++) {
				const base::OpCode opCode = it->id;
				switch (opCode) {
					case base::OpCode::INCR: // fallthrough
					case base::OpCode::DECR:
					{
						program.push_back(base::Operation(opCode));

						const Token& prev = *(it - 1);
						if (prev.id == base::OpCode::NAME) {
							program.push_back(base::Operation(base::OpCode::STORE, base::StackFrame(std::get<std::string>(prev.value))));
						}
					}
					break;
					case base::OpCode::ADD: // fallthrough
					case base::OpCode::SUB: // fallthrough
					case base::OpCode::MULT: // fallthrough
					case base::OpCode::DIV: // fallthrough
					case base::OpCode::EQ: // fallthrough
					case base::OpCode::UNEQ: // fallthrough
					case base::OpCode::BIGGER: // fallthrough
					case base::OpCode::LESS:
						program.push_back(base::Operation(opCode));
						break;
					case base::OpCode::LITERAL:
						program.push_back(base::Operation(base::OpCode::LOAD, base::StackFrame(base::BasicType(literalToValue(it->value)))));
						break;
					case base::OpCode::NAME:
						program.push_back(base::Operation(base::OpCode::LOAD, base::StackFrame(std::get<std::string>(it->value))));
						break;
				}
			}

			return program;
		}

		void insertCurlyBrackets(Iterator& begin, Iterator end) {
			const Iterator beginBody = unwindGroup(begin, end);

			program.push_back(base::Operation(base::OpCode::BEGIN_SCOPE));
			currentScopeLevel++;
			compileBlock(beginBody + 1, begin);
			currentScopeLevel--;
			program.push_back(base::Operation(base::OpCode::END_SCOPE));

			begin++;
		}

		void insertRoundBrackets(Iterator& begin, Iterator end) {
			const Iterator beginCondition = unwindGroup(begin, end);
			program.splice(program.end(), shuntingYard(beginCondition, begin));
			begin++;
		}

		void insert_if_base(Iterator& begin, Iterator end, size_t jumpToEndIndex) {
			assume_isOperator(begin->id, base::OpCode::BRACKET_ROUND_OPEN, begin->pos);
			insertRoundBrackets(begin, end);

			const size_t jumpOverIndex = nextIndex();
			program.emplace_back(base::OpCode::JUMP_LABEL_IF_NOT, base::StackFrame(base::BasicType(jumpOverIndex))); // jump over if

			assume_isOperator(begin->id, base::OpCode::BRACKET_CURLY_OPEN, begin->pos);
			insertCurlyBrackets(begin, end);

			program.emplace_back(base::OpCode::JUMP_LABEL, base::StackFrame(base::BasicType(jumpToEndIndex))); // jump to end
			program.emplace_back(base::OpCode::LABEL, base::StackFrame(base::BasicType(jumpOverIndex))); // after if label
		}

		void parse_if(Iterator& begin, Iterator end) {
			begin++; // "if"
			const size_t jumpToEndIndex = nextIndex();
			insert_if_base(begin, end, jumpToEndIndex);
			program.emplace_back(base::OpCode::LABEL, base::StackFrame(base::BasicType(jumpToEndIndex))); // set end label
		}

		void parse_else(Iterator& begin, Iterator end) {
			begin++; // "else"
			base::Operation jumpToEndLabel = program.back(); // retrieve 'else' end label
			assume_isOperator(jumpToEndLabel.getOpCode(), base::OpCode::LABEL, 0);
			program.pop_back();

			if (begin->id == base::OpCode::IF) {
				begin++;

				const size_t jumpToEndIndex = jumpToEndLabel.firstValue().getValue().getUint();
				insert_if_base(begin, end, jumpToEndIndex);
			} else {
				assume_isOperator(begin->id, base::OpCode::BRACKET_CURLY_OPEN, begin->pos);
				insertCurlyBrackets(begin, end);
			}

			program.push_back(std::move(jumpToEndLabel)); // put back 'else' end label
		}

		void parse_while(Iterator& begin, Iterator end) {
			begin++; // "while"

			const size_t headLabel = nextIndex();
			const size_t endLabel = nextIndex();

			program.push_back(base::Operation(base::OpCode::LABEL, base::StackFrame(base::BasicType(headLabel))));

			assume_isOperator(begin->id, base::OpCode::BRACKET_ROUND_OPEN, begin->pos);
			insertRoundBrackets(begin, end);

			program.push_back(base::Operation(base::OpCode::JUMP_LABEL_IF_NOT, base::StackFrame(base::BasicType(endLabel))));

			assume_isOperator(begin->id, base::OpCode::BRACKET_CURLY_OPEN, begin->pos);
			loopLabels.push({ headLabel, endLabel, currentScopeLevel });
			insertCurlyBrackets(begin, end);
			loopLabels.pop();

			program.push_back(base::Operation(base::OpCode::JUMP_LABEL, base::StackFrame(base::BasicType(headLabel))));

			program.push_back(base::Operation(base::OpCode::LABEL, base::StackFrame(base::BasicType(endLabel))));
		}

		void embeddCodeStatement(Iterator& begin, Iterator end) {
			const auto endStatementIt = std::find_if(begin, end, [](const Token& n) {
				return n.id == base::OpCode::END_STATEMENT;
			});
			assume(endStatementIt != end, "Missing end statement!", begin->pos);

			program.splice(program.end(), shuntingYard(begin, endStatementIt));
			begin = endStatementIt + 1;
		}


		void compileBlock(Iterator begin, Iterator end) {
			try {
				while (begin != end) {
					switch (begin->id) {
						case base::OpCode::TYPE:
						{
							base::StackFrame variableType(literalToValue(begin->value));
							begin++;
							base::StackFrame variableName(std::get<std::string>(begin->value));
							program.push_back(base::Operation(base::OpCode::CREATE, std::move(variableType), std::move(variableName)));
						}
						break;
						case base::OpCode::NAME:
							if ((begin + 1)->id == base::OpCode::ASSIGN) {
								std::string variableName = std::get<std::string>(begin->value);
								begin += 2;
								embeddCodeStatement(begin, end);
								program.push_back(base::Operation(base::OpCode::STORE, base::StackFrame(std::move(variableName))));
							} else {
								embeddCodeStatement(begin, end);
							}
							break;
						case base::OpCode::IF:
							parse_if(begin, end);
							break;
						case base::OpCode::ELSE:
							parse_else(begin, end);
							break;
						case base::OpCode::WHILE:
							parse_while(begin, end);
							break;
						case base::OpCode::CONTINUE:
							for (int i = 0; i < currentScopeLevel - loopLabels.top().scopeLevel; i++) {
								program.push_back(base::Operation(base::OpCode::END_SCOPE));
							}
							program.push_back(base::Operation(base::OpCode::JUMP_LABEL, base::StackFrame(base::BasicType(loopLabels.top().head))));
							begin++;
							assume(begin->id == base::OpCode::END_STATEMENT, "Missing end statement", begin->pos);
							begin++;
							break;
						case base::OpCode::BREAK:
							for (int i = 0; i < currentScopeLevel - loopLabels.top().scopeLevel; i++) {
								program.push_back(base::Operation(base::OpCode::END_SCOPE));
							}
							program.push_back(base::Operation(base::OpCode::JUMP_LABEL, base::StackFrame(base::BasicType(loopLabels.top().end))));
							begin++;
							assume(begin->id == base::OpCode::END_STATEMENT, "Missing end statement", begin->pos);
							begin++;
							break;
						case base::OpCode::BRACKET_CURLY_OPEN:
							insertCurlyBrackets(begin, end);
							break;
						default:
							embeddCodeStatement(begin, end);
					}
				}
			} catch (const ex::ParserException& ex) {
				std::cout << ex.what() << "\n" << source.markedLineAt(ex.getPos()) << std::endl;
				runToNextSync(begin, end);
				success = false;
			}
		}

	public:
		bool isSuccess() const {
			return success;
		}

		ShuntingYard(Iterator begin, Iterator end, const base::Source& source)
			: _current(begin), _end(end), source(source) {
		}

		ShuntingYard(const std::vector<Token>& nodes, const base::Source& source)
			: _current(nodes.begin()), _end(nodes.end()), source(source) {
		}

		std::list<base::Operation> run() {
			compileBlock(_current, _end);
			return std::move(program);
		}
	};
} // namespace parser