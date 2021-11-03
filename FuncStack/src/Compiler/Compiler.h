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
	class Compiler {
		Source source;

		Tokenizer tokenizer;

		ScopeDict scope;
		FunctionCache functions;

		Token currentToken;

		base::Program program;
		base::Bytecode& bytecode = program.bytecode;
		std::vector<base::BasicType>& literals = program.constants;

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

		const Token& peak() {
			return tokenizer.peak();
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
			const int priority = token.prio();
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

		void checkPlausibility(const std::vector<Token>& tokens) const {
			int score = 0;
			for (const Token& token : tokens) {
				score += opCodeImpact(token.opCode);
				assume(score >= 0, "Plausibility check failed", currentToken);
			}
			assume(score == 1, "Plausibility check failed", currentToken);
		}

		void shuntingYard(const std::vector<Token>& tokens) {
			std::stack<Token, std::vector<Token>> operatorStack;
			std::vector<Token> sortedTokens;

			for (auto it = tokens.begin(); it != tokens.end(); it++) {
				if (it->opCode == base::OpCode::COMMA) {
					continue;
				}

				if (it->opCode == base::OpCode::LOAD_LITERAL) {
					sortedTokens.push_back(*it);
					continue;
				}

				if (it->opCode == base::OpCode::NAME) {
					auto next = it + 1;
					if ((next != tokens.end()) and (next->opCode == base::OpCode::BRACKET_ROUND_OPEN)) {
						operatorStack.push(*it);
						it++;
						operatorStack.push(*it);
					} else {
						sortedTokens.push_back(*it);
					}
					continue;
				}

				if (base::isOpeningBracket(it->opCode)) {
					operatorStack.push(*it);
					continue;
				}

				if (base::isClosingBracket(it->opCode)) {
					auto [bracketOpen, bracketClose] = base::getBracketGroup(it->opCode);

					while (!operatorStack.empty() and (operatorStack.top().opCode != bracketOpen)) {
						assume(!base::isOpeningBracket(operatorStack.top().opCode), "Found closing bracket that doesnt match", *it);
						sortedTokens.push_back(top_and_pop(operatorStack));
					}
					assume(operatorStack.size() > 0, "Group didnt end", tokens.front());

					operatorStack.pop();

					if (operatorStack.top().opCode == base::OpCode::NAME) {
						sortedTokens.push_back(top_and_pop(operatorStack));
					}

					continue;
				}

				while (!operatorStack.empty() and (getCheckedPriority(operatorStack.top()) >= getCheckedPriority(*it))) {
					sortedTokens.push_back(top_and_pop(operatorStack));
				}
				operatorStack.push(*it);
			}

			while (!operatorStack.empty()) {
				sortedTokens.push_back(top_and_pop(operatorStack));
			}

			//checkPlausibility(sortedTokens);

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
						bytecode.push_back(base::Operation(base::OpCode::LOAD_LITERAL, program.addConstant(literalToValue(*it))));
						break;
					case base::OpCode::NAME:
					{
						const std::string& name = it->get<std::string>();
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
			const std::vector<Token> tokens = unwindGroup();
			shuntingYard(tokens);
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


		void parse_while() {
			assert(currentToken.opCode == base::OpCode::WHILE);

			const size_t headIndex = index();

			currentToken = tokenizer.next("Missing round brackets after 'while'", base::OpCode::BRACKET_ROUND_OPEN);
			insertRoundBrackets();

			bytecode.push_back(base::Operation(base::OpCode::JUMP_IF_NOT, 0));
			const size_t jumpIndex = index();

			scope.pushLoop();
			scope.pushScope();
			compileStatement(); // TODO Single statement need scope
			popScope();
			const std::vector<ScopeDict::Breaker> breakers = scope.popLoop();

			insertJump(base::OpCode::JUMP, index(), headIndex);
			const size_t indexAfterLoop = index();
			rewireJump(jumpIndex, indexAfterLoop);

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

		Function::Variable extractParameter() {
			assert(currentToken.opCode == base::OpCode::TYPE);
			const Token paramType = currentToken;

			currentToken = tokenizer.next(base::OpCode::NAME);
			const Token paramName = currentToken;

			currentToken = tokenizer.next(base::OpCode::COMMA, base::OpCode::BRACKET_ROUND_CLOSE);
			return Function::Variable{ static_cast<base::TypeIndex>(paramType.get<size_t>()), paramName.get<std::string>() };
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
			assume(scope.level() == 0, "Function declarations allowed only on global scope", currentToken); // TODO removed in future
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

			bool isNewFunction = functions.push(functionName.get<std::string>(), returnType, parameters, index());
			assume(isNewFunction, "Function already known", currentToken);

			scope.pushScope();
			for (const Function::Variable& var : parameters) { // TODO shitshow
				const bool unknownVariable = scope.pushVariable(var.name, static_cast<size_t>(var.type));
				assume(unknownVariable, "Variable already defined", currentToken);
			}

			compileStatement();
			popScope();

			bytecode.push_back(base::Operation(base::OpCode::END_FUNCTION));
			rewireJump(jumpIndex, index());
		}

		void parse_return() { // TODO better!
			assert(currentToken.opCode == base::OpCode::RETURN);
			currentToken = tokenizer.next();
			const uint32_t returnSize = currentToken.opCode == base::OpCode::END_STATEMENT ? 0 : 1;
			embeddCodeStatement();
			bytecode.push_back(base::Operation(base::OpCode::RETURN, returnSize));
		}

		void registerBreaker() {
			size_t levelsToJump = 1;
			assert((currentToken.opCode == base::OpCode::CONTINUE) or (currentToken.opCode == base::OpCode::BREAK));
			const Token breaker = currentToken;

			currentToken = tokenizer.next();
			if (currentToken.opCode != base::OpCode::END_STATEMENT) {
				assume(currentToken.opCode == base::OpCode::LOAD_LITERAL, "expected literal after keyword", currentToken);
				assume(std::holds_alternative<base::sm_int>(currentToken.value), "expected literal after keyword", currentToken);
				levelsToJump = std::get<base::sm_int>(currentToken.value);
				assume(levelsToJump > 0, "jump levels must be > 0", currentToken);
				currentToken = tokenizer.next(base::OpCode::END_STATEMENT);
			}

			bytecode.push_back(base::Operation(base::OpCode::POP, 0));
			bytecode.push_back(base::Operation(base::OpCode::JUMP, 0));
			scope.pushBreaker(index(), levelsToJump, breaker);

			assert(currentToken.opCode == base::OpCode::END_STATEMENT);
			currentToken = tokenizer.next();
		}

		void embeddCodeStatement() {
			std::vector<Token> tokens;
			while (!currentToken.isEnd() and (currentToken.opCode != base::OpCode::END_STATEMENT)) {
				tokens.push_back(currentToken);
				currentToken = tokenizer.next();
			}
			assume(!currentToken.isEnd(), "Missing end statement!", currentToken);

			if (tokens.size() > 0) {
				shuntingYard(tokens);
			}

			currentToken = tokenizer.next();
		}

		void compileStatement() {
			try {
				switch (currentToken.opCode) {
					case base::OpCode::TYPE:
					{
						assert(std::holds_alternative<std::size_t>(currentToken.value));
						size_t variableType = std::get<size_t>(currentToken.value);

						currentToken = tokenizer.next("Missing variable name after type declaration", base::OpCode::NAME);
						std::string variableName = std::get<std::string>(currentToken.value);

						const bool unknownVariable = scope.pushVariable(variableName, variableType);
						assume(unknownVariable, "Variable already defined", currentToken);
						bytecode.push_back(base::Operation(base::OpCode::CREATE_VARIABLE, variableType));
					}
					// fallthrough
					case base::OpCode::NAME:
						if (peak().opCode == base::OpCode::ASSIGN) {
							assert(std::holds_alternative<std::string>(currentToken.value));
							base::Operation storeOperation = scope.createStoreOperation(currentToken);
							currentToken = tokenizer.next(); currentToken = tokenizer.next();
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

			return std::move(program);
		}
	};
} // namespace parser