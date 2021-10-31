#pragma once

#include <string>
#include <cctype>
#include <functional>
#include <optional>
#include <sstream>

#include "Token.h"
#include "src/Base/Source.h"

namespace compiler {
	using ExtractorResult = std::optional<Token>;

	inline bool partOfVariableName(char c) {
		return std::isalnum(c) or (c == '_');
	};

	class Tokenizer {
		std::optional<Token> peaked;
		bool success = true;

		base::StringView view;

		void skipWhitespace() {
			while (!view.isEnd() and std::isspace(*view)) {
				view.removePrefix(1);
			}
		}

		std::string extractLexem(std::function<bool(char)> recognizer) {
			std::string currentToken;
			while (!view.isEnd() && recognizer(*view)) {
				currentToken += *view;
				view.removePrefix(1);
			}
			return currentToken;
		}

		std::string extractNumber() {
			std::string currentToken;

			while (!view.isEnd() and std::isdigit(*view)) {
				currentToken += *view;
				view.removePrefix(1);
			}

			return currentToken;
		}

		ExtractorResult extractName() {
			const size_t save = view.pos();

			if (!view.isEnd() and partOfVariableName(*view)) {
				const std::string lexem = extractLexem(partOfVariableName);

				if ((lexem == "true") or (lexem == "false")) {
					return Token(base::OpCode::LOAD_LITERAL, lexem == "true", view.pos()); // Literal Bool
				}

				const base::OpCode symbol = base::opCodeFromKeyword(lexem);
				if (symbol != base::OpCode::ERR) {
					return Token(symbol, view.pos()); // Type Keyword
				}

				const base::TypeIndex variableTypeId = base::stringToId(lexem);
				if (variableTypeId != base::TypeIndex::Err) {
					return Token(base::OpCode::TYPE, static_cast<size_t>(variableTypeId), view.pos());
				}

				return Token(base::OpCode::NAME, lexem, view.pos()); // Name
			}

			view.toPos(save);
			return {};
		}

		ExtractorResult extractLiteral() {
			size_t save = view.pos();

			if (!view.isEnd() and std::isdigit(*view)) {
				const std::string beforeDot = extractNumber();

				if (!view.isEnd() and (*view == '.')) {
					save = view.pos();
					view.removePrefix(1);

					if (!view.isEnd() and std::isdigit(*view)) {
						const std::string afterDot = extractNumber();
						if (!std::isalpha(*view)) {
							return Token(base::OpCode::LOAD_LITERAL, std::stod(beforeDot + "." + afterDot), view.pos()); // Literal Double
						}
					}
					view.toPos(save);
				}

				if (!view.isEnd() and (*view == 'u')) {
					view.removePrefix(1);
					return Token(base::OpCode::LOAD_LITERAL, static_cast<base::sm_uint>(std::stoul(beforeDot)), view.pos()); // Literal Long
				}
				return Token(base::OpCode::LOAD_LITERAL, static_cast<base::sm_int>(std::stol(beforeDot)), view.pos()); // Literal Long
			}

			view.toPos(save);
			return {};
		}

		ExtractorResult extractOperator() {
			const size_t save = view.pos();

			if (!view.isEnd()) {
				if (view.length() > 1) {
					const base::StringView op = view.subStr(2);
					const base::OpCode symbol = base::opCodeFromSymbol(op);
					if (symbol != base::OpCode::ERR) {
						view.removePrefix(2);
						return Token(symbol, view.pos());
					}
				}

				const base::StringView op = view.subStr(1);
				const base::OpCode symbol = base::opCodeFromSymbol(op);
				if (symbol != base::OpCode::ERR) {
					view.removePrefix(1);
					return Token(symbol, view.pos());
				}
			}

			view.toPos(save);
			return {};
		}

		Token extract() {
			if (view.isEnd()) {
				return Token(base::OpCode::END_PROGRAM, view.pos());
			}

			skipWhitespace();

			if (view.isEnd()) {
				return Token(base::OpCode::END_PROGRAM, view.pos());
			}

			ExtractorResult result = extractLiteral();
			if (result.has_value()) {
				return result.value();
			}

			result = extractName();
			if (result.has_value()) {
				return result.value();
			}

			result = extractOperator();
			if (result.has_value()) {
				return result.value();
			}

			success = false;
			const size_t errorPos = view.pos();
			view.removePrefix(1);
			throw ex::ParserException("Could not recognize token", errorPos);
		}

		Token extract(std::string&& message, base::OpCode hint) {
			if (view.isEnd()) {
				return Token(base::OpCode::END_PROGRAM, view.pos());
			}

			skipWhitespace();
			if (view.isEnd()) {
				return Token(base::OpCode::END_PROGRAM, view.pos());
			}

			ExtractorResult result;
			switch (hint) {
				case base::OpCode::END_STATEMENT:
				case base::OpCode::COMMA:
				case base::OpCode::BRACKET_ROUND_OPEN:
				case base::OpCode::BRACKET_ROUND_CLOSE:
				case base::OpCode::BRACKET_CURLY_OPEN:
				case base::OpCode::BRACKET_CURLY_CLOSE:
				case base::OpCode::BRACKET_SQUARE_OPEN:
				case base::OpCode::BRACKET_SQUARE_CLOSE:
				case base::OpCode::EQ:
				case base::OpCode::UNEQ:
				case base::OpCode::BIGGER:
				case base::OpCode::LESS:
				case base::OpCode::ADD:
				case base::OpCode::SUB:
				case base::OpCode::MULT:
				case base::OpCode::DIV:
				case base::OpCode::INCR:
				case base::OpCode::DECR:
				case base::OpCode::ASSIGN:
					result = extractOperator();
					break;
				case base::OpCode::IF:
				case base::OpCode::ELSE:
				case base::OpCode::WHILE:
				case base::OpCode::CONTINUE:
				case base::OpCode::BREAK:
				case base::OpCode::TYPE:
				case base::OpCode::NAME:
				case base::OpCode::FUNC:
					result = extractName();
					break;
				case base::OpCode::LOAD_LITERAL:
					result = extractLiteral();
					break;
			}

			if (!result.has_value()) {
				success = false;
				const size_t errorPos = view.pos();
				view.removePrefix(1);
				throw ex::ParserException(message, errorPos);
			}
			return result.value();
		}

		Token nextToken() {
			if (!peaked.has_value()) {
				return extract();
			}

			Token t = std::move(peaked.value());
			peaked.reset();
			return t;
		}

		Token nextToken(std::string&& message, base::OpCode hint) {
			if (!peaked.has_value()) {
				return extract(std::move(message), hint);
			}

			Token t = std::move(peaked.value());
			peaked.reset();
			return t;
		}

		void assume(bool condition, const std::string& message, const Token& t) const {
			if (!condition) {
				throw ex::ParserException(message, t.pos);
			}
		}

	public:
		Tokenizer(const std::string& source) :
			view(source) {
		}

		bool isSuccess() const {
			return success;
		}

		void synchronize() {
			while (!view.isEnd() and utils::noneOf(*view, ';', ')', '}', ']')) {
				view.removePrefix(1);
			}
		}

		const Token& peak() {
			if (!peaked.has_value()) {
				peaked = extract();
			}
			return peaked.value();
		}

		Token next() {
			return nextToken();
		}

		Token next(std::string&& message, base::OpCode hint) {
			return nextToken(std::move(message), hint);
		}

		Token next(base::OpCode hint) {
			return nextToken("Expected: " + base::opCodeNameUser(hint), hint);
		}

		template<typename... T>
		Token next(std::string&& message, T&&... expectedOpCode) {
			Token token = next();
			assume(!token.isEnd() and utils::anyOf(token.opCode, expectedOpCode...), message, token);
			return token;
		}

		template<typename... T>
		Token next(const char* message, T&&... expectedOpCode) {
			Token token = next();
			assume(!token.isEnd() and utils::anyOf(token.opCode, expectedOpCode...), message, token);
			return token;
		}

		template<typename... T>
		Token next(T&&... expectedOpCode) {
			return next("Expected any of: " + base::opCodeNameUser(expectedOpCode...), expectedOpCode...);
		}
	};
}