#pragma once

#include <algorithm>
#include <sstream>
#include <stack>
#include <list>
#include <vector>

#include "ScopeDict.h"
#include "Token.h"
#include "Function.h"
#include "Tokenizer.h"

#include "src/Utils/Source.h"
#include "src/Base/Program.h"

namespace compiler {
	template<base::OpCode op>
	bool isOpCode(const Token& t) {
		return t.opCode == op;
	}

	class Compiler {
		using TokenList = std::vector<Token>;
		using Iterator = std::vector<Token>::const_iterator;

		Source source;

		Tokenizer tokenizer;

		ScopeDict scope;
		FunctionCache functions;

		Token currentToken;

		base::Program program;
		base::Bytecode& bytecode = program.bytecode;

		bool success = true;

		size_t index() const {
			return bytecode.size() - 1;
		}

		void popScope() {
			const uint32_t variablePopCount = scope.popScope();
			if (variablePopCount > 0) {
				bytecode.push_back(base::Operation(base::OpCode::POP, variablePopCount));
			}
		}

		void rewireJump(size_t from, size_t to) {
			base::Operation& jump = bytecode[from];
			assume(anyOf(jump.getOpCode(), base::OpCode::JUMP, base::OpCode::JUMP_IF_NOT, base::OpCode::CALL_FUNCTION), "expected to rewire jump", currentToken);
			int32_t jumpDistance = to - from;
			if (jumpDistance < 0) jumpDistance--;
			jump.signedData() = jumpDistance;
		}

		void insertJump(base::OpCode jump, size_t from, size_t to) {
			assume(anyOf(jump, base::OpCode::JUMP, base::OpCode::JUMP_IF_NOT, base::OpCode::CALL_FUNCTION), "expected to rewire jump", currentToken);
			int32_t jumpDistance = to - from;
			if (jumpDistance < 0) jumpDistance--;
			bytecode.push_back(base::Operation(jump, jumpDistance));
		}

		void assume(bool condition, const std::string& message, const Token& t) const {
			if (!condition) {
				throw ex::ParserException(message, t.pos);
			}
		}

		void synchronize(const ex::ParserException& ex) {
			std::cout << ex.what() << "\n" << source.markedLineAt(ex.getPos()) << std::endl;
			success = false;
			tokenizer.synchronize();
			currentToken = tokenizer.next();
		}

		int getCheckedPriority(const Token& token) const {
			int priority = token.prio();
			assume(priority != -1, "OpCode has no priority: " + opCodeName(token.opCode), token);
			return priority;
		}

		std::vector<Token> unwindGroup() {
			assert(anyOf(currentToken.opCode, base::OpCode::BRACKET_ROUND_OPEN, base::OpCode::BRACKET_CURLY_OPEN, base::OpCode::BRACKET_SQUARE_OPEN));
			auto [bracketBegin, bracketEnd] = base::getBracketGroup(currentToken.opCode);

			std::vector<Token> tokens;
			int counter = 0;
			const Token groupStart = currentToken;
			do {
				assume(!currentToken.isEnd(), "Group didnt close", groupStart);

				if (currentToken.opCode == bracketBegin) counter++;
				else if (currentToken.opCode == bracketEnd) counter--;
				else tokens.push_back(currentToken);
				currentToken = tokenizer.next();
			} while (counter > 0);
			return tokens;
		}

		std::vector<Token> unwindExpressionStatement() {
			std::vector<Token> tokens;
			while (!currentToken.isEnd() and (currentToken.opCode != base::OpCode::END_STATEMENT)) {
				tokens.push_back(currentToken);
				currentToken = tokenizer.next();
			}
			currentToken = tokenizer.next(); // ";"
			return tokens;
		}

		void checkPlausibility(const std::vector<Token>& tokens) const {
			int score = 0;
			for (const Token& token : tokens) {
				score += opCodeImpact(token.opCode);
				assume(score >= 0, "Plausibility check failed", currentToken);
			}
			assume(score == 1, "Plausibility check failed", currentToken);
		}

		[[nodiscard]] TokenList shuntingYard(Iterator begin, Iterator end) {
			Iterator current = begin;
			std::stack<Token, std::vector<Token>> operatorStack;
			TokenList sortedTokens;

			for (; current != end; current++) {
				if (current->opCode == base::OpCode::COMMA) {
					continue;
				}

				if (current->opCode == base::OpCode::LOAD_LITERAL) {
					sortedTokens.push_back(*current);
					continue;
				}

				if (current->opCode == base::OpCode::NAME) {
					auto next = current + 1;
					if ((next != end) and (next->opCode == base::OpCode::BRACKET_ROUND_OPEN)) {
						operatorStack.push(*current);
						current++;
						operatorStack.push(*current);
					} else {
						sortedTokens.push_back(*current);
					}
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
						sortedTokens.push_back(top_and_pop(operatorStack));
					}
					assume(operatorStack.size() > 0, "Group didnt end", *begin);

					operatorStack.pop();

					if (operatorStack.top().opCode == base::OpCode::NAME) {
						sortedTokens.push_back(top_and_pop(operatorStack));
					}

					continue;
				}

				while (!operatorStack.empty() and (getCheckedPriority(operatorStack.top()) >= getCheckedPriority(*current))) {
					sortedTokens.push_back(top_and_pop(operatorStack));
				}
				operatorStack.push(*current);
			}

			while (!operatorStack.empty()) {
				sortedTokens.push_back(top_and_pop(operatorStack));
			}

			return sortedTokens;
		}

		void insertSortedTokens(const TokenList& sortedTokens) {
			for (auto it = sortedTokens.begin(); it != sortedTokens.end(); it++) {
				const base::OpCode opCode = it->opCode;
				switch (opCode) {
					case base::OpCode::INCR: // ->
					case base::OpCode::DECR:
					{
						bytecode.push_back(base::Operation(opCode));

						const auto prev = it - 1;
						if (prev->opCode == base::OpCode::NAME) {
							bytecode.push_back(scope.createStoreOperation(*prev));
						}
					}
					break;
					case base::OpCode::ADD: // ->
					case base::OpCode::SUB: // ->
					case base::OpCode::MULT: // ->
					case base::OpCode::DIV: // ->
					case base::OpCode::EQ: // ->
					case base::OpCode::UNEQ: // ->
					case base::OpCode::BIGGER: // ->
					case base::OpCode::LESS:
						bytecode.push_back(base::Operation(opCode));
						break;
					case base::OpCode::LOAD_LITERAL:
						bytecode.push_back(base::Operation(base::OpCode::LOAD_LITERAL, it->getNumber()));
						break;
					case base::OpCode::NAME:
					{
						const std::string& name = it->getString();
						if (functions.has(name)) {
							insertJump(base::OpCode::CALL_FUNCTION, index(), functions.offset(name).value());
							bytecode.back().side_unsignedData() = functions.paramCount(name);
						} else {
							bytecode.push_back(scope.createLoadOperation(*it));
						}
					}
					break;
					default:
						throw ex::ParserException("Unknown token after shunting yard", it->pos);
				}
			}
		}

		std::vector<size_t> expectedType(const TokenList& sortedTokens) const {
			std::stack<size_t, std::vector<size_t>> opStack;
			for (const Token& t : sortedTokens) {
				switch (t.opCode) {
					case base::OpCode::LOAD_LITERAL:
						opStack.push(static_cast<size_t>(tokenizer.literals.get(t.getNumber()).typeId()));
						break;
					case base::OpCode::NAME:
						opStack.push(scope.typeOf(t));
						break;
					case base::OpCode::ADD:
					case base::OpCode::SUB:
					case base::OpCode::MULT:
					case base::OpCode::DIV:
					{
						const size_t type1 = top_and_pop(opStack);
						const size_t type2 = top_and_pop(opStack);
						assume(type1 == type2, "Incomplete types during verification", t);
						opStack.push(type1);
						break;
					}
					case base::OpCode::INCR:
					case base::OpCode::DECR:
						break; // type stays the same
					case base::OpCode::BIGGER:
					case base::OpCode::LESS:
					case base::OpCode::EQ:
					case base::OpCode::UNEQ:
					{
						const size_t type1 = top_and_pop(opStack);
						const size_t type2 = top_and_pop(opStack);
						assume(type1 == type2, "Incomplete types during verification", t);
						opStack.push(static_cast<size_t>(base::TypeIndex::Bool));
						break;
					}
					default:
						throw ex::ParserException("Unknown type", t.pos);
				}
			}

			std::vector<size_t> types;
			while (!opStack.empty()) {
				types.push_back(top_and_pop(opStack));
			}
			return types;
		}

		void insertCurlyBrackets() {
			scope.pushScope();
			assert(currentToken.opCode == base::OpCode::BRACKET_CURLY_OPEN);
			currentToken = tokenizer.next();

			while (noneOf(currentToken.opCode, base::OpCode::END_PROGRAM, base::OpCode::BRACKET_CURLY_CLOSE)) {
				compileStatement();
			}

			popScope();

			assert(currentToken.opCode == base::OpCode::BRACKET_CURLY_CLOSE);
			currentToken = tokenizer.next();
		}

		void insertRoundBrackets() {
			assert(currentToken.opCode == base::OpCode::BRACKET_ROUND_OPEN);
			const std::vector<Token> tokens = unwindGroup();
			insertSortedTokens(shuntingYard(tokens.begin(), tokens.end()));
		}

		template<base::OpCode jump>
		size_t insert_if_base() {
			static_assert(anyOf(jump, base::OpCode::JUMP, base::OpCode::JUMP_IF_NOT));

			bytecode.push_back(base::Operation(jump, 0)); // jump over block
			const size_t jumpIndex = index();
			scope.pushScope();
			compileStatement();
			popScope();
			rewireJump(jumpIndex, index());
			return jumpIndex;
		}

		void parse_if() {
			assert(currentToken.opCode == base::OpCode::IF);

			currentToken = tokenizer.next("Missing round brackets after 'if'", base::OpCode::BRACKET_ROUND_OPEN);
			insertRoundBrackets();

			const size_t jumpIndex = insert_if_base<base::OpCode::JUMP_IF_NOT>();

			if (!currentToken.isEnd() and (currentToken.opCode == base::OpCode::ELSE)) {
				currentToken = tokenizer.next();
				rewireJump(jumpIndex, index() + 1);
				insert_if_base<base::OpCode::JUMP>();
			}
		}

		void insertLoop(std::optional<std::pair<Iterator, Iterator>> initialization, std::pair<Iterator, Iterator> condition, std::optional<std::pair<Iterator, Iterator>> iteration) {
			scope.pushScope();

			if (initialization.has_value()) {
				embeddExpression(initialization->first, initialization->second); // <- initialization
			}

			const size_t headIndex = index();
			const TokenList sortedTokens = shuntingYard(condition.first, condition.second);
			const std::vector<size_t> typeResults = expectedType(sortedTokens);
			assume(typeResults.size() == 1, "Condition needs to have exactly one result", *condition.first);
			assume(typeResults.front() == static_cast<size_t>(base::TypeIndex::Bool), "Condition needs boolean as result type", *condition.first);
			insertSortedTokens(sortedTokens); // <- condition

			bytecode.push_back(base::Operation(base::OpCode::JUMP_IF_NOT, 0));
			const size_t jumpIndex = index();

			scope.pushLoop();
			compileStatement();

			if (iteration.has_value()) {
				insertSortedTokens(shuntingYard(iteration->first, iteration->second)); // <- iteration
			}
			const std::vector<ScopeDict::Breaker> breakers = scope.popLoop();

			insertJump(base::OpCode::JUMP, index(), headIndex);
			const size_t indexAfterLoop = index();
			rewireJump(jumpIndex, indexAfterLoop);

			popScope();

			for (const ScopeDict::Breaker& breaker : breakers) {
				switch (breaker.token.opCode) {
					case base::OpCode::CONTINUE:
						rewireJump(breaker.index, headIndex + 1); /* "pc++" */
						break;
					case base::OpCode::BREAK:
						rewireJump(breaker.index, indexAfterLoop);
						break;
					default:
						throw ex::ParserException("Unexpected breaker", breaker.token.pos);
				}

				base::Operation& endScopeOp = bytecode[breaker.index - 1];
				assert(endScopeOp.getOpCode() == base::OpCode::POP);
				const int32_t levelsToBreak = breaker.level - scope.level();
				assume(levelsToBreak > 0, "Too many levels to break", currentToken);
				endScopeOp.signedData() = breaker.variables - scope.sizeLocalVariables();
			}
		}

		void parse_while() {
			assert(currentToken.opCode == base::OpCode::WHILE);

			const size_t headIndex = index();

			currentToken = tokenizer.next("Missing round brackets after 'while'", base::OpCode::BRACKET_ROUND_OPEN);
			const TokenList condition = unwindGroup();
			assume(condition.size() > 0, "Missing condition in while-loop", currentToken);
			assume(condition.front().opCode != base::OpCode::TYPE, "No declaration in this section allowed", condition.front());
			insertLoop({}, std::make_pair(condition.begin(), condition.end()), {});
		}

		void parse_for() {
			assert(currentToken.opCode == base::OpCode::FOR);

			currentToken = tokenizer.next("Missing round brackets after 'while'", base::OpCode::BRACKET_ROUND_OPEN);
			const std::vector<Token> loopHead = unwindGroup();

			const Iterator beginInitialization = loopHead.begin();
			const Iterator endInitialization = std::find_if(loopHead.begin(), loopHead.end(), isOpCode<base::OpCode::END_STATEMENT>);
			assume(endInitialization != loopHead.end(), "Missing condition section in for-loop head", currentToken);

			const Iterator beginCondition = endInitialization + 1;
			const Iterator endCondition = std::find_if(beginCondition, loopHead.end(), isOpCode<base::OpCode::END_STATEMENT>);
			assume(beginCondition != endCondition, "Missing condition in while-loop", *beginCondition);
			assume(beginCondition->opCode != base::OpCode::TYPE, "No declaration in this section allowed", *beginCondition);
			assume(endCondition != loopHead.end(), "Missing iterative section in for-loop head", currentToken);

			const Iterator beginIteration = endCondition + 1;
			assume(beginIteration->opCode != base::OpCode::TYPE, "No declaration in this section allowed", *beginIteration);
			const Iterator endIteration = loopHead.end();

			insertLoop(std::make_pair(beginInitialization, endInitialization), std::make_pair(beginCondition, endCondition), std::make_pair(beginIteration, endIteration));
		}

		Function::Variable extractParameter() {
			assert(currentToken.opCode == base::OpCode::TYPE);
			const Token paramType = currentToken;

			currentToken = tokenizer.next(base::OpCode::NAME);
			const Token paramName = currentToken;

			currentToken = tokenizer.next(base::OpCode::COMMA, base::OpCode::BRACKET_ROUND_CLOSE);
			return Function::Variable{ static_cast<base::TypeIndex>(paramType.getNumber()), paramName.getString() };
		}

		std::vector<Function::Variable> extractFunctionParameters() {
			assert(currentToken.opCode == base::OpCode::BRACKET_ROUND_OPEN);
			currentToken = tokenizer.next(base::OpCode::TYPE, base::OpCode::BRACKET_ROUND_CLOSE);

			std::vector<Function::Variable> parameters;
			if (currentToken.opCode == base::OpCode::TYPE) {
				parameters.push_back(extractParameter());

				while (currentToken.opCode == base::OpCode::COMMA) {
					currentToken = tokenizer.next(base::OpCode::TYPE);
					parameters.push_back(extractParameter());
				}
			}

			assert(currentToken.opCode == base::OpCode::BRACKET_ROUND_CLOSE);
			currentToken = tokenizer.next();
			return parameters;
		}

		void parse_function() { // TODO optimize
			assert(currentToken.opCode == base::OpCode::FUNC);
			assume(scope.level() == 0, "Function declarations allowed only on global scope", currentToken); // removed in future
			currentToken = tokenizer.next(base::OpCode::TYPE, base::OpCode::NAME);

			std::optional<base::TypeIndex> returnType;
			if (currentToken.opCode == base::OpCode::TYPE) {
				returnType = static_cast<base::TypeIndex>(std::get<size_t>(currentToken.value));
				currentToken = tokenizer.next("Expected function name after declaration", base::OpCode::NAME);
			}

			const Token functionName = currentToken;

			currentToken = tokenizer.next(base::OpCode::BRACKET_ROUND_OPEN);
			std::vector<Function::Variable> parameters = extractFunctionParameters();

			bytecode.push_back(base::Operation(base::OpCode::JUMP, 0));
			const size_t jumpIndex = index();

			bool isNewFunction = functions.push(functionName.getString(), returnType, parameters, index());
			assume(isNewFunction, "Function already known", currentToken);

			scope.pushScope();
			for (const Function::Variable& var : parameters) {
				const bool unknownVariable = scope.pushVariable(var.name, static_cast<size_t>(var.type));
				assume(unknownVariable, "Variable already defined", currentToken);
			}

			compileStatement();
			popScope();

			bytecode.push_back(base::Operation(base::OpCode::END_FUNCTION));
			rewireJump(jumpIndex, index());
		}

		void parse_return() {
			assert(currentToken.opCode == base::OpCode::RETURN);
			currentToken = tokenizer.next();
			const uint32_t returnSize = currentToken.opCode == base::OpCode::END_STATEMENT ? 0 : 1; // returns more than one possibly in future

			const std::vector<Token> returnExpression = unwindExpressionStatement();
			embeddExpression(returnExpression.begin(), returnExpression.end());

			bytecode.push_back(base::Operation(base::OpCode::RETURN, returnSize));
		}

		void registerBreaker() {
			size_t levelsToJump = 1;
			assert((currentToken.opCode == base::OpCode::CONTINUE) or (currentToken.opCode == base::OpCode::BREAK));
			const Token breaker = currentToken;

			currentToken = tokenizer.next();
			if (currentToken.opCode != base::OpCode::END_STATEMENT) {
				assume(currentToken.opCode == base::OpCode::LOAD_LITERAL, "expected literal after keyword", currentToken);
				assume(currentToken.hasNumber(), "expected literal after keyword", currentToken);
				levelsToJump = tokenizer.literals.get(currentToken.getNumber()).getInt();
				assume(levelsToJump > 0, "jump levels must be > 0", currentToken);
				currentToken = tokenizer.next(base::OpCode::END_STATEMENT);
			}

			bytecode.push_back(base::Operation(base::OpCode::POP, 0));
			bytecode.push_back(base::Operation(base::OpCode::JUMP, 0));
			scope.pushBreaker(index(), levelsToJump, breaker);

			assert(currentToken.opCode == base::OpCode::END_STATEMENT);
			currentToken = tokenizer.next();
		}

		void variableDeclaration(Iterator& begin) {
			assert(std::holds_alternative<std::size_t>(begin->value));
			const size_t variableType = std::get<size_t>(begin->value);

			begin++;
			assume(begin->opCode == base::OpCode::NAME, "Missing variable name after type declaration", *begin);
			const std::string variableName = std::get<std::string>(begin->value);

			const bool unknownVariable = scope.pushVariable(variableName, variableType);
			assume(unknownVariable, "Variable already defined", *begin);
			bytecode.push_back(base::Operation(base::OpCode::CREATE_VARIABLE, variableType));

		}

		void embeddExpression(Iterator begin, Iterator end) {
			if (begin->opCode == base::OpCode::TYPE) {
				variableDeclaration(begin);
			}

			Iterator next = begin + 1;
			if (next->opCode == base::OpCode::ASSIGN) {
				const Token variableName = *begin;
				begin++;
				assume(begin->opCode == base::OpCode::ASSIGN, "Expected assignment after variable name", *begin);
				begin++;
				insertSortedTokens(shuntingYard(begin, end));
				bytecode.push_back(scope.createStoreOperation(variableName));
			} else {
				insertSortedTokens(shuntingYard(begin, end));
			}
		}

		void compileStatement() {
			try {
				switch (currentToken.opCode) {
					case base::OpCode::TYPE:
					case base::OpCode::NAME:
					{
						const std::vector<Token> returnExpression = unwindExpressionStatement();
						embeddExpression(returnExpression.begin(), returnExpression.end());
						break;
					}
					case base::OpCode::IF:
						parse_if();
						break;
					case base::OpCode::WHILE:
						parse_while();
						break;
					case base::OpCode::FOR:
						parse_for();
						break;
					case base::OpCode::FUNC:
						parse_function();
						break;
					case base::OpCode::RETURN:
						parse_return();
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
					case base::OpCode::END_STATEMENT:
						currentToken = tokenizer.next();
						break;
					default:
						throw ex::ParserException("Unknown token", currentToken.pos);
				}
			} catch (const ex::ParserException& ex) {
				synchronize(ex);
			}
		}

	public:
		bool isSuccess() const {
			return success;
		}

		Compiler(Source source)
			: source(std::move(source)), tokenizer(this->source.str()), currentToken(tokenizer.next()) {
		}

		base::Program run() {
			try {
				while (!currentToken.isEnd()) {
					try {
						switch (currentToken.opCode) {
							case base::OpCode::TYPE:
							case base::OpCode::FUNC:
								compileStatement();
								break;
							default:
								throw ex::ParserException("Only declarations allowed on global scope", currentToken.pos);
						}
					} catch (const ex::ParserException& ex) {
						synchronize(ex);
					}
				}

				if (success) {
					const std::optional<size_t> mainPosition = functions.offset("main", {}, {});
					assume(mainPosition.has_value(), "No main function in code", currentToken);

					insertJump(base::OpCode::CALL_FUNCTION, index(), mainPosition.value());
					bytecode.back().side_unsignedData() = 0;

					bytecode.push_back(base::Operation(base::OpCode::END_PROGRAM));
				}
			} catch (const ex::ParserException& ex) {
				synchronize(ex);
			}

			program.literals = std::move(tokenizer.literals);
			return std::move(program);
		}
	};
} // namespace parser