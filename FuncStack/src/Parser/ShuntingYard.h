#pragma once

#include <algorithm>
#include <sstream>
#include <stack>
#include <list>
#include <vector>

#include "ScopeDict.h"

#include "src/Base/Source.h"
#include "src/Base/Program.h"

namespace compiler {
	class ShuntingYard {
		using Iterator = std::vector<Token>::const_iterator;

		const base::Source& source;
		const Iterator _begin;
		const Iterator _end;
		Iterator _current;

		ScopeDict scope;

		base::Program program;
		std::vector<base::Operation>& bytecode = program.bytecode;
		std::vector<base::BasicType>& literals = program.constants;

		bool success = true;

		size_t index() const {
			return bytecode.size() - 1;
		}

		void assume(bool condition, const std::string& message, const Token& t) const {
			if (!condition) {
				throw ex::ParserException(message, t.pos);
			}
		}

		void runToNextSync(Iterator& current, Iterator end) {
			while ((current != end) and (!utils::any_of(current->opCode, { base::OpCode::BRACKET_ROUND_CLOSE, base::OpCode::BRACKET_CURLY_CLOSE }))) {
				current++;
			}
			current++;
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

		std::vector<base::Operation> shuntingYard(Iterator current, Iterator end) {
			const Iterator this_current_copy = current;

			std::stack<Token, std::vector<Token>> operatorStack;
			std::vector<Token> sortedTokens;

			for (; current != end; current++) {
				if ((current->opCode == base::OpCode::LOAD_LITERAL) or (current->opCode == base::OpCode::NAME)) {
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

			std::vector<base::Operation> subProgram;
			for (auto it = sortedTokens.begin(); it != sortedTokens.end(); it++) {
				const base::OpCode opCode = it->opCode;
				switch (opCode) {
					case base::OpCode::INCR: // fallthrough
					case base::OpCode::DECR:
					{
						subProgram.push_back(base::Operation(opCode));

						const Iterator prev = it - 1;
						if (prev->opCode == base::OpCode::NAME) {
							subProgram.push_back(scope.createStoreOperation(prev));
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
						subProgram.push_back(base::Operation(opCode));
						break;
					case base::OpCode::LOAD_LITERAL:
						subProgram.push_back(base::Operation(base::OpCode::LOAD_LITERAL, program.addConstant(literalToValue(it))));
						break;
					case base::OpCode::NAME:
					{
						subProgram.push_back(scope.createLoadOperation(it));
					}
					break;
					default:
						throw ex::ParserException("Unknown token after shunting yard", current->pos);
				}
			}

			return subProgram;
		}

		void insertCurlyBrackets(Iterator& current, Iterator end) {
			program.bytecode.push_back(base::Operation(base::OpCode::BEGIN_SCOPE));
			scope.pushScope();
			current++;

			while ((current != end) and (current->opCode != base::OpCode::BRACKET_CURLY_CLOSE)) {
				compileStatement(current, end);
			}

			scope.popScope();
			program.bytecode.push_back(base::Operation(base::OpCode::END_SCOPE, 1));

			current++;
		}

		void insertRoundBrackets(Iterator& current, Iterator end) {
			const Iterator beginCondition = unwindGroup(current, end);
			program.spliceBytecode(shuntingYard(beginCondition, current));
			current++;
		}

		template<base::OpCode jump>
		size_t insert_if_base(Iterator& current, Iterator end) {
			static_assert((jump == base::OpCode::JUMP) or (jump == base::OpCode::JUMP_IF_NOT));

			bytecode.push_back(base::Operation(jump, 0)); // jump over block
			const size_t jumpIndex = index();
			compileStatement(current, end);
			bytecode[jumpIndex].signedData() = index() - jumpIndex;
			return jumpIndex;
		}

		void parse_if(Iterator& current, Iterator end) {
			current++; // "if"

			assume(current->opCode == base::OpCode::BRACKET_ROUND_OPEN, "Missing round brackets after 'if'", *current);
			insertRoundBrackets(current, end);

			const size_t jumpIndex = insert_if_base<base::OpCode::JUMP_IF_NOT>(current, end);

			if ((current != end) and (current->opCode == base::OpCode::ELSE)) {
				current++; // "else"
				bytecode[jumpIndex].signedData()++;
				insert_if_base<base::OpCode::JUMP>(current, end);
			}
		}


		void parse_while(Iterator& current, Iterator end) {
			current++; // "while"

			const size_t headIndex = index();
			assume(current->opCode == base::OpCode::BRACKET_ROUND_OPEN, "Missing round brackets after 'while'", *current);
			insertRoundBrackets(current, end);

			program.bytecode.push_back(base::Operation(base::OpCode::JUMP_IF_NOT, 0));
			const size_t jumpIndex = index();

			scope.pushLoop();
			compileStatement(current, end); // TODO Single statement need scope
			const std::vector<ScopeDict::Breaker> breakers = scope.popLoop();

			program.bytecode.push_back(base::Operation(base::OpCode::JUMP, static_cast<int32_t>(headIndex - index() - 1 /* "pc++" */)));
			const size_t indexAfterLoop = index();
			bytecode[jumpIndex].signedData() = indexAfterLoop - jumpIndex;

			for (const ScopeDict::Breaker& breaker : breakers) {
				switch (breaker.it->opCode) {
					case base::OpCode::CONTINUE:
						bytecode[breaker.index].signedData() = headIndex - breaker.index /* "pc++" */;
						break;
					case base::OpCode::BREAK:
						bytecode[breaker.index].signedData() = indexAfterLoop - breaker.index;
						break;
					default:
						throw ex::ParserException("Unexpected breaker", breaker.it->pos);
				}

				base::Operation& endScopeOp = bytecode[breaker.index - 1];
				assume(endScopeOp.getOpCode() == base::OpCode::END_SCOPE, "No 'END_SCOPE' after breaker", *breaker.it);
				const int32_t levelsToBreak = breaker.level - scope.level();
				assume(levelsToBreak > 0, "Too many levels to break: " + std::to_string(levelsToBreak), *breaker.it);
				endScopeOp.signedData() = levelsToBreak;
			}
		}

		template<base::OpCode breaker>
		void registerBreaker(Iterator& current) {
			static_assert((breaker == base::OpCode::CONTINUE) or (breaker == base::OpCode::BREAK));

			const Iterator breakerIt = current;
			size_t levelsToJump = 1;
			current++; // breaker
			if (current->opCode != base::OpCode::END_STATEMENT) {
				assume(current->opCode == base::OpCode::LOAD_LITERAL, "expected number after keyword", *current);
				assume(std::holds_alternative<base::sm_int>(current->value), "expected number after keyword", *current);
				levelsToJump = std::get<base::sm_int>(current->value);
				assume(levelsToJump > 0, "jump levels must be > 0", *current);
				current++; // literal
			}

			program.bytecode.push_back(base::Operation(base::OpCode::END_SCOPE, 0));
			program.bytecode.push_back(base::Operation(base::OpCode::JUMP, 0));
			scope.pushBreaker(index(), levelsToJump, breakerIt);

			current++; // ";"
			bytecode.push_back(base::Operation(base::OpCode::POP));
		}

		void execute_continue(Iterator& current) {
			registerBreaker<base::OpCode::CONTINUE>(current);
		}

		void execute_break(Iterator& current) {
			registerBreaker<base::OpCode::BREAK>(current);
		}

		void embeddCodeStatement(Iterator& current, Iterator end) {
			const auto endStatementIt = std::find_if(current, end, [](const Token& n) {
				return n.opCode == base::OpCode::END_STATEMENT;
			});
			assume(endStatementIt != end, "Missing end statement!", *current);

			program.spliceBytecode(shuntingYard(current, endStatementIt));
			current = endStatementIt + 1;
		}

		void compileStatement(Iterator& current, Iterator end) {
			try {
				switch (current->opCode) {
					case base::OpCode::TYPE:
					{
						assert(std::holds_alternative<std::size_t>(current->value));
						size_t variableType = std::get<size_t>(current->value);
						current++;

						assume(std::holds_alternative<std::string>(current->value), "Missing variable name after type declaration", *current);
						std::string variableName = std::get<std::string>(current->value);
						const bool unknownVariable = scope.pushVariable(variableName);
						assume(unknownVariable, "Variable already defined", *current);

						program.bytecode.push_back(base::Operation(base::OpCode::CREATE_VARIABLE, variableType));
					}
					break;
					case base::OpCode::NAME:
						if ((current + 1)->opCode == base::OpCode::ASSIGN) {
							assert(std::holds_alternative<std::string>(current->value));
							base::Operation storeOperation = scope.createStoreOperation(current);
							current += 2;
							embeddCodeStatement(current, end);
							program.bytecode.push_back(std::move(storeOperation));

						} else {
							embeddCodeStatement(current, end);
						}
						break;
					case base::OpCode::IF:
						parse_if(current, end);
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
			: _begin(nodes.begin()), _current(nodes.begin()), _end(nodes.end()), source(source) {
		}

		base::Program run() {
			while (_current != _end) {
				compileStatement(_current, _end);
			}

			program.bytecode.push_back(base::Operation(base::OpCode::END_PROGRAM));
			return std::move(program);
		}
	};
} // namespace parser