#pragma once

#include <algorithm>
#include <sstream>
#include <stack>
#include <list>
#include <vector>

#include "ScopeDict.h"

#include "src/Base/Source.h"

namespace compiler {
	class ShuntingYard {
		using Iterator = std::vector<Token>::const_iterator;

		const base::Source& source;
		const Iterator _end;
		Iterator _current;

		ScopeDict scope;

		std::list<base::Operation> program;
		bool success = true;

		void assume(bool condition, const std::string& message, const Token& t) const {
			if (!condition) {
				throw ex::ParserException(message, t.pos);
			}
		}

		size_t nextIndex() const {
			static size_t index = 0;
			return index++;
		}

		void runToNextSync(Iterator& current, Iterator end) {
			while ((current != end) and (utils::any_of(current->opCode, { base::OpCode::END_STATEMENT, base::OpCode::BRACKET_ROUND_CLOSE, base::OpCode::BRACKET_CURLY_CLOSE }))) {
				current++;
			}
		}

		int getCheckedPriority(const Token& token) const {
			const int priority = token.prio();
			assume(priority != -1, "OpCode has no priority: " + opCodeName(token.opCode), token);
			return priority;
		}

		base::BasicType literalToValue(Iterator current) const {
			if (std::holds_alternative<base::sm_int>(current->value)) {
				return base::BasicType(std::get<base::sm_int>(current->value));
			} else if (std::holds_alternative<base::sm_uint>(current->value)) {
				return base::BasicType(std::get<base::sm_uint>(current->value));
			} else if (std::holds_alternative<base::sm_float>(current->value)) {
				return base::BasicType(std::get<base::sm_float>(current->value));
			} else if (std::holds_alternative<base::sm_bool>(current->value)) {
				return base::BasicType(std::get<base::sm_bool>(current->value));
			}
			ex::ParserException("Unknown value", current->pos);
		}

		Iterator unwindGroup(Iterator& current, Iterator end) {
			const Iterator groupStart = current + 1;

			auto [bracketBegin, bracketEnd] = base::getBracketGroup(current->opCode);
			assume(bracketBegin != base::OpCode::ERR, "Expected group", *current);

			int counter = 1;
			do {
				current++;
				assume(current != end, "Group didnt close", *groupStart);

				if (current->opCode == bracketBegin) counter++;
				if (current->opCode == bracketEnd) counter--;
			} while (counter > 0);
			return groupStart;
		}

		void checkPlausibility(Iterator current, const std::vector<Token>& tokens) const {
			int score = 0;
			for (const Token& token : tokens) {
				score += opCodeImpact(token.opCode);
				assume(score >= 0, "Plausibility check failed", *current);
			}
			assume(score == 1, "Plausibility check failed", *current);
		}

		std::list<base::Operation> shuntingYard(Iterator current, Iterator end) const {
			const Iterator this_current_copy = current;

			std::stack<Token, std::vector<Token>> operatorStack;
			std::vector<Token> sortedTokens;

			for (; current != end; current++) {
				if ((current->opCode == base::OpCode::LITERAL) or (current->opCode == base::OpCode::NAME)) {
					sortedTokens.push_back(*current);
					continue;
				}

				if (base::isOpeningBracket(current->opCode)) {
					operatorStack.push(*current);
					continue;
				}

				if (base::isClosingBracket(current->opCode)) {
					auto [bracketOpen, bracketClose] = base::getBracketGroup(current->opCode);

					while (!operatorStack.empty() and (operatorStack.top().opCode != bracketOpen)) {
						assume(!base::isOpeningBracket(operatorStack.top().opCode), "Found closing bracket that doesnt match", *current);
						sortedTokens.push_back(operatorStack.top());
						operatorStack.pop();
					}
					assume(operatorStack.size() > 0, "Group didnt end", *this_current_copy);

					operatorStack.pop();
					// TODO As soon as there are functions implemented
					// if (operatorStack.top() is function) {
					//		sortedTokens.push_back(operatorStack.top());
					//		operatorStack.pop();
					// }
					continue;
				}

				while (!operatorStack.empty() and (getCheckedPriority(operatorStack.top()) >= getCheckedPriority(*current))) {
					sortedTokens.push_back(operatorStack.top());
					operatorStack.pop();
				}
				operatorStack.push(*current);
			}

			while (!operatorStack.empty()) {
				sortedTokens.push_back(operatorStack.top());
				operatorStack.pop();
			}

			checkPlausibility(current, sortedTokens);

			std::list<base::Operation> program;
			for (auto it = sortedTokens.begin(); it != sortedTokens.end(); it++) {
				const base::OpCode opCode = it->opCode;
				switch (opCode) {
					case base::OpCode::INCR: // fallthrough
					case base::OpCode::DECR:
					{
						program.push_back(base::Operation(opCode));

						const Iterator prev = it - 1;
						if (prev->opCode == base::OpCode::NAME) {
							program.push_back(scope.createStoreOperation(prev));
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
						program.push_back(base::Operation(base::OpCode::LITERAL, base::BasicType(literalToValue(it))));
						break;
					case base::OpCode::NAME:
					{
						program.push_back(scope.createLoadOperation(it));
					}
					break;
					default:
						throw ex::ParserException("Unknown token after shunting yard", current->pos);
				}
			}

			return program;
		}

		void insertCurlyBrackets(Iterator& current, Iterator end) {
			const Iterator beginBody = unwindGroup(current, end);

			program.push_back(base::Operation(base::OpCode::BEGIN_SCOPE));
			scope.pushScope();
			compileBlock(beginBody, current);
			scope.popScope();
			program.push_back(base::Operation(base::OpCode::END_SCOPE));

			current++;
		}

		void insertRoundBrackets(Iterator& current, Iterator end) {
			const Iterator beginCondition = unwindGroup(current, end);
			program.splice(program.end(), shuntingYard(beginCondition, current));
			current++;
		}

		void insert_if_base(Iterator& current, Iterator end, size_t jumpToEndIndex) {
			assume(current->opCode == base::OpCode::BRACKET_ROUND_OPEN, "Missing round brackets after 'if'", *current);
			insertRoundBrackets(current, end);

			const size_t jumpOverIndex = nextIndex();
			program.emplace_back(base::OpCode::JUMP_LABEL_IF_NOT, base::BasicType(jumpOverIndex)); // jump over if

			assume(current->opCode == base::OpCode::BRACKET_CURLY_OPEN, "Missing 'if' body", *current);
			insertCurlyBrackets(current, end);

			program.emplace_back(base::OpCode::JUMP_LABEL, base::BasicType(jumpToEndIndex)); // jump to end
			program.emplace_back(base::OpCode::LABEL, base::BasicType(jumpOverIndex)); // after if label
		}

		void parse_if(Iterator& current, Iterator end) {
			current++; // "if"
			const size_t jumpToEndIndex = nextIndex();
			insert_if_base(current, end, jumpToEndIndex);
			program.emplace_back(base::OpCode::LABEL, base::BasicType(jumpToEndIndex)); // set end label
		}

		void parse_else(Iterator& current, Iterator end) {
			current++; // "else"
			base::Operation jumpToEndLabel = program.back(); // retrieve 'else' end label
			assume(jumpToEndLabel.getOpCode() == base::OpCode::LABEL, "'else' declaration without previous 'if'", *current);
			program.pop_back();

			if (current->opCode == base::OpCode::IF) {
				current++;

				const size_t jumpToEndIndex = jumpToEndLabel.value().getUint();
				insert_if_base(current, end, jumpToEndIndex);
			} else {
				assume(current->opCode == base::OpCode::BRACKET_CURLY_OPEN, "'else' missing body", *current);
				insertCurlyBrackets(current, end);
			}

			program.push_back(std::move(jumpToEndLabel)); // put back 'else' end label
		}

		void parse_while(Iterator& current, Iterator end) {
			current++; // "while"

			const size_t headLabel = nextIndex();
			const size_t endLabel = nextIndex();

			program.push_back(base::Operation(base::OpCode::LABEL, base::BasicType(headLabel)));

			assume(current->opCode == base::OpCode::BRACKET_ROUND_OPEN, "Missing round brackets after 'while'", *current);
			insertRoundBrackets(current, end);

			program.push_back(base::Operation(base::OpCode::JUMP_LABEL_IF_NOT, base::BasicType(endLabel)));

			assume(current->opCode == base::OpCode::BRACKET_CURLY_OPEN, "'while' missing body", *current);
			scope.pushLoop(headLabel, endLabel);
			insertCurlyBrackets(current, end);
			scope.popLoop();

			program.push_back(base::Operation(base::OpCode::JUMP_LABEL, base::BasicType(headLabel)));

			program.push_back(base::Operation(base::OpCode::LABEL, base::BasicType(endLabel)));
		}

		void jumpOutOfLoop(Iterator& current, std::function<size_t(const ScopeDict::LoopLayer&)> which) {
			size_t levelsToJump = 0;
			const Iterator next = current + 1;
			if (next->opCode == base::OpCode::LITERAL) {
				assume(std::holds_alternative<base::sm_int>(next->value), "expected number after keyword", *current);
				levelsToJump = std::get<base::sm_int>(next->value) - 1;
				assume(levelsToJump > 0, "jump levels must be > 0", *current);
				current++;
			}

			const auto loopLabels = scope.loopLabelFromOffset(levelsToJump);
			assume(loopLabels.has_value(), "cannot jump to outer loop, there is no", *current);
			current++;

			for (int i = 0; i < scope.level() - loopLabels.value().level; i++) {
				program.push_back(base::Operation(base::OpCode::END_SCOPE));
			}

			program.push_back(base::Operation(base::OpCode::JUMP_LABEL, base::BasicType(which(loopLabels.value()))));
			assume(current->opCode == base::OpCode::END_STATEMENT, "Missing end statement", *current);
			current++;
		}

		void execute_continue(Iterator& current) {
			jumpOutOfLoop(current, [](const ScopeDict::LoopLayer& l) {
				return l.head;
			});
		}

		void execute_break(Iterator& current) {
			jumpOutOfLoop(current, [](const ScopeDict::LoopLayer& l) {
				return l.end;
			});
		}

		void embeddCodeStatement(Iterator& current, Iterator end) {
			const auto endStatementIt = std::find_if(current, end, [](const Token& n) {
				return n.opCode == base::OpCode::END_STATEMENT;
			});
			assume(endStatementIt != end, "Missing end statement!", *current);

			program.splice(program.end(), shuntingYard(current, endStatementIt));
			current = endStatementIt + 1;
		}

		void compileBlock(Iterator current, Iterator end) {
			try {
				while (current != end) {
					switch (current->opCode) {
						case base::OpCode::TYPE:
						{
							assert(!std::holds_alternative<std::monostate>(current->value));
							base::BasicType variableType = literalToValue(current);
							current++;

							assume(std::holds_alternative<std::string>(current->value), "Missing variable name after type declaration", *current);
							std::string variableName = std::get<std::string>(current->value);
							const bool unknownVariable = scope.pushVariable(variableName);
							assume(unknownVariable, "Variable already defined", *current);

							program.push_back(base::Operation(base::OpCode::CREATE, std::move(variableType)));
						}
						break;
						case base::OpCode::NAME:
							if ((current + 1)->opCode == base::OpCode::ASSIGN) {
								assert(std::holds_alternative<std::string>(current->value));
								base::Operation storeOperation = scope.createStoreOperation(current);
								current += 2;
								embeddCodeStatement(current, end);
								program.push_back(std::move(storeOperation));
							} else {
								embeddCodeStatement(current, end);
							}
							break;
						case base::OpCode::IF:
							parse_if(current, end);
							break;
						case base::OpCode::ELSE:
							parse_else(current, end);
							break;
						case base::OpCode::WHILE:
							parse_while(current, end);
							break;
						case base::OpCode::CONTINUE:
							execute_continue(current);
							break;
						case base::OpCode::BREAK:
							execute_break(current);
							break;
						case base::OpCode::BRACKET_CURLY_OPEN:
							insertCurlyBrackets(current, end);
							break;
						case base::OpCode::BRACKET_ROUND_OPEN:
							insertRoundBrackets(current, end);
							break;
						default:
							throw ex::ParserException("Unknown token", current->pos);
					}
				}
			} catch (const ex::ParserException& ex) {
				std::cout << ex.what() << "\n" << source.markedLineAt(ex.getPos()) << std::endl;
				runToNextSync(current, end);
				success = false;
			}
		}

	public:
		bool isSuccess() const {
			return success;
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