#pragma once

#include <algorithm>
#include <sstream>
#include <stack>
#include <list>
#include <vector>

#include "Token.h"

#include "src/Exception.h"
#include "src/Base/Source.h"

namespace compiler {
	class ScopeDict {
		using Iterator = std::vector<Token>::const_iterator;

	public:
		struct LoopLayer {
			size_t level, head, end;
		};

		struct Variable {
			size_t level;
			std::string name;
		};

		ScopeDict() = default;

		void pushScope() {
			scopeLevel++;
		}

		void popScope() {
			const auto pos = std::find_if(variables.begin(), variables.end(), [=](const Variable& v) {
				return v.level == scopeLevel;
			});
			variables.erase(pos, variables.end());
			scopeLevel--;
		}

		void pushLoop(size_t headLabel, size_t endLabel) {
			loops.push_back({ scopeLevel, headLabel, endLabel });
		}

		void popLoop() {
			loops.pop_back();
		}

		bool pushVariable(const std::string& variableName) {
			if (hasVariable(variableName)) {
				return false;
			}

			variables.push_back({ scopeLevel, variableName });
			return true;
		}

		std::optional<size_t> offsetVariable(const std::string& variableName) const {
			const auto pos = _getVariableIt(variableName);
			if (pos != variables.end()) {
				return std::distance(variables.begin(), pos);
			}
			return {};
		}

		bool hasVariable(const std::string& variableName) const {
			return _getVariableIt(variableName) != variables.end();
		}

		std::optional<LoopLayer> loopLabelFromOffset(size_t offset) const {
			if (loops.size() < offset) {
				return {};
			}

			return loops[loops.size() - 1 - offset];
		}

		size_t level() const {
			return scopeLevel;
		}

		base::Operation createStoreOperation(Iterator it) const {
			const std::optional<size_t> variableOffset = offsetVariable(std::get<std::string>(it->value));
			assume(variableOffset.has_value(), "Variable not declared", it);
			return base::Operation(base::OpCode::STORE, base::StackFrame(base::BasicType(variableOffset.value())));
		}

		base::Operation createLoadOperation(Iterator it) const {
			const std::optional<size_t> variableOffset = offsetVariable(std::get<std::string>(it->value));
			assume(variableOffset.has_value(), "used variable not declared", it);
			return base::Operation(base::OpCode::LOAD, base::StackFrame(base::BasicType(variableOffset.value())));
		}

	private:
		std::vector<LoopLayer> loops;
		std::vector<Variable> variables;
		size_t scopeLevel = 0;

		const std::vector<Variable>::const_iterator _getVariableIt(const std::string& name) const {
			return std::find_if(variables.begin(), variables.end(), [&](const Variable& v) {
				return v.name == name;
			});
		}

		void assume(bool condition, const std::string& message, Iterator it) const {
			if (!condition) {
				throw ex::ParserException(message, it->pos);
			}
		}
	};

	class ShuntingYard {
		using Iterator = std::vector<Token>::const_iterator;

		const base::Source& source;
		const Iterator _end;
		Iterator _current;

		ScopeDict scope;

		std::list<base::Operation> program;
		bool success = true;

		void assume(bool condition, const std::string& message, size_t pos) const {
			if (!condition) {
				throw ex::ParserException(message, pos);
			}
		}

		void assume(bool condition, const std::string& message, const Token& t) const {
			assume(condition, message, t.pos);
		}

		void assume(bool condition, const std::string& message, std::vector<Token>::const_iterator it) const {
			assume(condition, message, it->pos);
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

						const Iterator prev = it - 1;
						if (prev->id == base::OpCode::NAME) {
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
						program.push_back(base::Operation(base::OpCode::LITERAL, base::StackFrame(base::BasicType(literalToValue(it->value)))));
						break;
					case base::OpCode::NAME:
					{
						program.push_back(scope.createLoadOperation(it));
					}
						break;
				}
			}

			return program;
		}

		void insertCurlyBrackets(Iterator& begin, Iterator end) {
			const Iterator beginBody = unwindGroup(begin, end);

			program.push_back(base::Operation(base::OpCode::BEGIN_SCOPE));
			scope.pushScope();
			compileBlock(beginBody + 1, begin);
			scope.popScope();
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

				const size_t jumpToEndIndex = jumpToEndLabel.value().getValue().getUint();
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
			scope.pushLoop(headLabel, endLabel);
			insertCurlyBrackets(begin, end);
			scope.popLoop();

			program.push_back(base::Operation(base::OpCode::JUMP_LABEL, base::StackFrame(base::BasicType(headLabel))));

			program.push_back(base::Operation(base::OpCode::LABEL, base::StackFrame(base::BasicType(endLabel))));
		}

		void jumpOutOfLoop(Iterator& current, std::function<size_t(const ScopeDict::LoopLayer&)> which) {
			size_t levelsToJump = 0;
			const Iterator next = current + 1;
			if (next->id == base::OpCode::LITERAL) {
				assume(std::holds_alternative<base::sm_int>(next->value), "expected number after keyword", current);
				levelsToJump = std::get<base::sm_int>(next->value) - 1;
				assume(levelsToJump > 0, "jump levels must be > 0", current);
				current++;
			}
			
			const auto loopLabels = scope.loopLabelFromOffset(levelsToJump);
			assume(loopLabels.has_value(), "cannot jump to outer loop, there is no", current);
			current++;

			for (int i = 0; i < scope.level() - loopLabels.value().level; i++) {
				program.push_back(base::Operation(base::OpCode::END_SCOPE));
			}

			program.push_back(base::Operation(base::OpCode::JUMP_LABEL, base::StackFrame(base::BasicType(which(loopLabels.value())))));
			assume(current->id == base::OpCode::END_STATEMENT, "Missing end statement", current);
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
							assert(!std::holds_alternative<std::monostate>(begin->value));
							base::StackFrame variableType(literalToValue(begin->value));
							begin++;
							
							assume(std::holds_alternative<std::string>(begin->value), "Missing variable name after type declaration", begin);
							std::string variableName = std::get<std::string>(begin->value);
							const bool unknownVariable = scope.pushVariable(variableName);
							assume(unknownVariable, "Variable already defined", begin->pos);

							program.push_back(base::Operation(base::OpCode::CREATE, std::move(variableType)));
						}
						break;
						case base::OpCode::NAME:
							if ((begin + 1)->id == base::OpCode::ASSIGN) {
								assert(std::holds_alternative<std::string>(begin->value));
								base::Operation storeOperation = scope.createStoreOperation(begin);
								begin += 2;
								embeddCodeStatement(begin, end);
								program.push_back(std::move(storeOperation));
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
							execute_continue(begin);
							break;
						case base::OpCode::BREAK:
							execute_break(begin);
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