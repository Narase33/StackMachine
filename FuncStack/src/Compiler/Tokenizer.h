#pragma once

#include <string>
#include <cctype>
#include <functional>
#include <optional>
#include <sstream>

#include "Token.h"
#include "src/Utils/Source.h"

namespace compiler {
	using ExtractorResult = std::optional<Token>;

	inline bool partOfVariableName(char c) {
		return std::isalnum(c) or (c == '_');
	};

	class Tokenizer {
		std::optional<Token> peaked;
		bool success = true;

		StringWindow view;

		void skipWhitespace() {
			while (!view.isEnd() and std::isspace(*view)) {
				view.removePrefix(1);
			}
		}

		using Recognizer = bool(*)(char);
		StringWindow extractLexem(Recognizer recognizer) {
			int i = 0;
			for (; i < view.length(); i++) {
				if (!recognizer(view[i])) {
					break;
				}
			}
			return view.subStr(i);
		}

		StringWindow extractNumber() {
			return extractLexem([](char c) -> bool {
				return std::isdigit(c);
			});
		}

		ExtractorResult extractType(const StringWindow& nameLexem) {
			const base::TypeIndex variableTypeId = base::stringToId(nameLexem);
			if (variableTypeId != base::TypeIndex::Err) {
				return Token(base::OpCode::TYPE, static_cast<size_t>(variableTypeId), view.pos()); // Type
			}
			return {};
		}

		ExtractorResult extractKeyword(const StringWindow& nameLexem) {
			const base::OpCode symbol = base::opCodeFromKeyword(nameLexem);
			if (symbol != base::OpCode::ERR) {
				return Token(symbol, view.pos()); // Keyword
			}
			return {};
		}

		ExtractorResult extractName(const StringWindow& nameLexem) {
			if (nameLexem.length() > 0) {
				return Token(base::OpCode::NAME, nameLexem.str(), view.pos()); // Name
			}
			return {};
		}

		ExtractorResult extractLiteral() {
			const size_t pos = view.pos();
			if (view.isEnd()) {
				return {};
			}

			if (view.startsWith("true")) {
				view.removePrefix(4);
				return Token(base::OpCode::LOAD_LITERAL, true, pos); // Literal Bool
			}

			if (view.startsWith("false")) {
				view.removePrefix(5);
				return Token(base::OpCode::LOAD_LITERAL, false, pos); // Literal Bool
			}

			if (std::isdigit(*view)) {
				StringWindow number = extractNumber(); // TODO Better
				view.removePrefix(number.length());

				if (!view.isEnd() and (*view == '.')) {
					number.addSuffix(1);
					view.removePrefix(1);

					if (!view.isEnd() and std::isdigit(*view)) {
						const StringWindow digitsAfterDot = extractNumber();
						number.addSuffix(digitsAfterDot.length());
						view.removePrefix(digitsAfterDot.length());

						if (!std::isalpha(*view)) {
							return Token(base::OpCode::LOAD_LITERAL, std::stod(number.str()), pos); // Literal Double
						}
					}

					return {};
				}

				if (!view.isEnd() and (*view == 'u')) {
					view.removePrefix(1);
					return Token(base::OpCode::LOAD_LITERAL, static_cast<base::sm_uint>(std::stoul(number.str())), pos); // Literal Long
				}

				return Token(base::OpCode::LOAD_LITERAL, static_cast<base::sm_int>(std::stol(number.str())), pos); // Literal Long
			}

			return {};
		}

		ExtractorResult extractOperator() {
			if (!view.isEnd()) {
				if (view.length() > 1) {
					const StringWindow op = view.subStr(2);
					const base::OpCode symbol = base::opCodeFromSymbol(op);
					if (symbol != base::OpCode::ERR) {
						view.removePrefix(2);
						return Token(symbol, view.pos());
					}
				}

				const StringWindow op = view.subStr(1);
				const base::OpCode symbol = base::opCodeFromSymbol(op);
				if (symbol != base::OpCode::ERR) {
					view.removePrefix(1);
					return Token(symbol, view.pos());
				}
			}

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

			result = extractOperator();
			if (result.has_value()) {
				return result.value();
			}

			// name lexems ->
			const StringWindow nameLexem = extractLexem(partOfVariableName);
			view.removePrefix(nameLexem.length());

			result = extractKeyword(nameLexem);
			if (result.has_value()) {
				return result.value();
			}

			result = extractType(nameLexem);
			if (result.has_value()) {
				return result.value();
			}

			result = extractName(nameLexem);
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
				case base::OpCode::FUNC:
					result = extractKeyword(extractLexem(partOfVariableName));
					break;
				case base::OpCode::TYPE:
					result = extractType(extractLexem(partOfVariableName));
					break;
				case base::OpCode::NAME:
					result = extractName(extractLexem(partOfVariableName));
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
			while (!view.isEnd() and noneOf(*view, ';', ')', '}', ']')) {
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
			assume(!token.isEnd() and anyOf(token.opCode, expectedOpCode...), message, token);
			return token;
		}

		template<typename... T>
		Token next(const char* message, T&&... expectedOpCode) {
			Token token = next();
			assume(!token.isEnd() and anyOf(token.opCode, expectedOpCode...), message, token);
			return token;
		}

		template<typename... T>
		Token next(T&&... expectedOpCode) {
			return next("Expected any of: " + base::opCodeNameUser(expectedOpCode...), expectedOpCode...);
		}
	};
}