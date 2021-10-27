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

		void consume(base::OpCode expected, const std::string& message) {
			assume((_current != _end) and (_current->opCode == expected), message, *_current);
			_current++;
		}

		void consume(base::OpCode expected) {
			consume(expected, "Expected " + opCodeName(expected) + ", got " + opCodeName(_current->opCode));
		}

		void assume(bool condition, const std::string& message, const Token& t) const {
			if (!condition) {
				throw ex::ParserException(message, t.pos);
			}
		}

		void runToNextSync(Iterator& _current, Iterator _end) {
			while ((_current != _end) and (!utils::any_of(_current->opCode, { base::OpCode::BRACKET_ROUND_CLOSE, base::OpCode::BRACKET_CURLY_CLOSE }))) {
				_current++;
			}
			_current++;
		}

		int getCheckedPriority(const Token& token) const {
			const int priority = token.prio();
			assume(priority != -1, "OpCode has no priority: " + opCodeName(token.opCode), token);
			return priority;
		}

		base::BasicType literalToValue(Iterator _current) const {
			if (std::holds_alternative<base::sm_int>(_current->value)) {
				return base::BasicType(std::get<base::sm_int>(_current->value));
			} else if (std::holds_alternative<base::sm_uint>(_current->value)) {
				return base::BasicType(std::get<base::sm_uint>(_current->value));
			} else if (std::holds_alternative<base::sm_float>(_current->value)) {
				return base::BasicType(std::get<base::sm_float>(_current->value));
			} else if (std::holds_alternative<base::sm_bool>(_current->value)) {
				return base::BasicType(std::get<base::sm_bool>(_current->value));
			}
			ex::ParserException("Unknown value", _current->pos);
		}

		Iterator unwindGroup() {
			const Iterator groupStart = _current + 1;

			auto [bracketBegin, bracketEnd] = base::getBracketGroup(_current->opCode);
			assume(bracketBegin != base::OpCode::ERR, "Expected group", *_current);

			int counter = 1;
			do {
				_current++;
				assume(_current != _end, "Group didnt close", *groupStart);

				if (_current->opCode == bracketBegin) counter++;
				if (_current->opCode == bracketEnd) counter--;
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

		void insertCurlyBrackets() {
			bytecode.push_back(base::Operation(base::OpCode::BEGIN_SCOPE));
			scope.pushScope();
			consume(base::OpCode::BRACKET_CURLY_OPEN);

			while ((_current != _end) and (_current->opCode != base::OpCode::BRACKET_CURLY_CLOSE)) {
				compileStatement();
			}

			scope.popScope();
			bytecode.push_back(base::Operation(base::OpCode::END_SCOPE, 1));

			consume(base::OpCode::BRACKET_CURLY_CLOSE);
		}

		void insertRoundBrackets() {
			const Iterator beginCondition = unwindGroup();
			program.spliceBytecode(shuntingYard(beginCondition, _current));
			consume(base::OpCode::BRACKET_ROUND_CLOSE);
		}

		template<base::OpCode jump>
		size_t insert_if_base() {
			static_assert((jump == base::OpCode::JUMP) or (jump == base::OpCode::JUMP_IF_NOT));

			bytecode.push_back(base::Operation(jump, 0)); // jump over block
			const size_t jumpIndex = index();
			compileStatement();
			bytecode[jumpIndex].signedData() = index() - jumpIndex;
			return jumpIndex;
		}

		void parse_if() {
			consume(base::OpCode::IF);

			assume(_current->opCode == base::OpCode::BRACKET_ROUND_OPEN, "Missing round brackets after 'if'", *_current);
			insertRoundBrackets();

			const size_t jumpIndex = insert_if_base<base::OpCode::JUMP_IF_NOT>();

			if ((_current != _end) and (_current->opCode == base::OpCode::ELSE)) {
				consume(base::OpCode::ELSE);
				bytecode[jumpIndex].signedData()++;
				insert_if_base<base::OpCode::JUMP>();
			}
		}


		void parse_while() {
			consume(base::OpCode::WHILE);

			const size_t headIndex = index();
			assume(_current->opCode == base::OpCode::BRACKET_ROUND_OPEN, "Missing round brackets after 'while'", *_current);
			insertRoundBrackets();

			bytecode.push_back(base::Operation(base::OpCode::JUMP_IF_NOT, 0));
			const size_t jumpIndex = index();

			scope.pushLoop();
			compileStatement(); // TODO Single statement need scope
			const std::vector<ScopeDict::Breaker> breakers = scope.popLoop();

			bytecode.push_back(base::Operation(base::OpCode::JUMP, static_cast<int32_t>(headIndex - index() - 1 /* "pc++" */)));
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

		void registerBreaker() {
			const Iterator breakerIt = _current;
			size_t levelsToJump = 1;
			_current++; // "continue" or "break"

			if (_current->opCode != base::OpCode::END_STATEMENT) {
				assume(_current->opCode == base::OpCode::LOAD_LITERAL, "expected literal after keyword", *_current);
				assume(std::holds_alternative<base::sm_int>(_current->value), "expected literal after keyword", *_current);
				levelsToJump = std::get<base::sm_int>(_current->value);
				assume(levelsToJump > 0, "jump levels must be > 0", *_current);
				consume(base::OpCode::LOAD_LITERAL);
			}

			bytecode.push_back(base::Operation(base::OpCode::END_SCOPE, 0));
			bytecode.push_back(base::Operation(base::OpCode::JUMP, 0));
			scope.pushBreaker(index(), levelsToJump, breakerIt);

			consume(base::OpCode::END_STATEMENT);
		}

		void embeddCodeStatement() {
			const auto endStatementIt = std::find_if(_current, _end, [](const Token& n) {
				return n.opCode == base::OpCode::END_STATEMENT;
			});
			assume(endStatementIt != _end, "Missing end statement!", *_current);

			program.spliceBytecode(shuntingYard(_current, endStatementIt));
			_current = endStatementIt + 1;
		}

		void compileStatement() {
			try {
				switch (_current->opCode) {
					case base::OpCode::TYPE:
					{
						assert(std::holds_alternative<std::size_t>(_current->value));
						size_t variableType = std::get<size_t>(_current->value);
						consume(base::OpCode::TYPE);

						assume(std::holds_alternative<std::string>(_current->value), "Missing variable name after type declaration", *_current);
						std::string variableName = std::get<std::string>(_current->value);
						const bool unknownVariable = scope.pushVariable(variableName);
						assume(unknownVariable, "Variable already defined", *_current);

						bytecode.push_back(base::Operation(base::OpCode::CREATE_VARIABLE, variableType));
					}
					break;
					case base::OpCode::NAME:
						if ((_current + 1)->opCode == base::OpCode::ASSIGN) {
							assert(std::holds_alternative<std::string>(_current->value));
							base::Operation storeOperation = scope.createStoreOperation(_current);
							_current += 2;
							embeddCodeStatement();
							bytecode.push_back(std::move(storeOperation));

						} else {
							embeddCodeStatement();
						}
						break;
					case base::OpCode::IF:
						parse_if();
						break;
					case base::OpCode::WHILE:
						parse_while();
						break;
					case base::OpCode::CONTINUE: // fallthrough
					case base::OpCode::BREAK:
						registerBreaker();
						break;
					case base::OpCode::BRACKET_CURLY_OPEN:
						insertCurlyBrackets();
						break;
					case base::OpCode::BRACKET_ROUND_OPEN:
						insertRoundBrackets();
						break;
					default:
						throw ex::ParserException("Unknown token", _current->pos);
				}
			} catch (const ex::ParserException& ex) {
				std::cout << ex.what() << "\n" << source.markedLineAt(ex.getPos()) << std::endl;
				runToNextSync(_current, _end);
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
				compileStatement();
			}

			bytecode.push_back(base::Operation(base::OpCode::END_PROGRAM));
			return std::move(program);
		}
	};
} // namespace parser