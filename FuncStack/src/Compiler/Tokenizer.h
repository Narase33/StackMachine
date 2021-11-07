#pragma once

#include <string>
#include <cctype>
#include <functional>
#include <algorithm>
#include <optional>
#include <sstream>

#include "Token.h"
#include "src/Base/LiteralStore.h"
#include "src/Utils/Source.h"

namespace compiler {
	using ExtractorResult = std::optional<Token>;

	inline bool partOfVariableName(char c) {
		return std::isalnum(c) or (c == '_');
	};

	class Tokenizer {
		std::optional<Token> peaked;

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
				return (c >= '0') and (c <= '9'); // faster than std::isdigit
			});
		}

		ExtractorResult extractType(const StringWindow& nameLexem) {
			const base::TypeIndex variableTypeId = base::stringToId(nameLexem);
			if (variableTypeId != base::TypeIndex::Err) {
				view.removePrefix(nameLexem.length());
				return Token(base::OpCode::TYPE, static_cast<size_t>(variableTypeId), view.pos()); // Type
			}
			return {};
		}

		ExtractorResult extractKeyword(const StringWindow& nameLexem) {
			const base::OpCode symbol = base::opCodeFromKeyword(nameLexem);
			if (symbol != base::OpCode::ERR) {
				view.removePrefix(nameLexem.length());
				return Token(symbol, view.pos()); // Keyword
			}
			return {};
		}

		ExtractorResult extractName(const StringWindow& nameLexem) {
			if (nameLexem.length() > 0) {
				view.removePrefix(nameLexem.length());
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
				return Token(base::OpCode::LOAD_LITERAL, literals.push(true), pos); // Literal Bool
			}

			if (view.startsWith("false")) {
				view.removePrefix(5);
				return Token(base::OpCode::LOAD_LITERAL, literals.push(false), pos); // Literal Bool
			}

			if (std::isdigit(*view)) {
				StringWindow number = extractNumber();
				view.removePrefix(number.length());

				if (!view.isEnd() and (*view == '.')) {
					number.addSuffix(1);
					view.removePrefix(1);

					if (!view.isEnd() and std::isdigit(*view)) {
						const StringWindow digitsAfterDot = extractNumber();
						number.addSuffix(digitsAfterDot.length());
						view.removePrefix(digitsAfterDot.length());

						if (!std::isalpha(*view)) {
							return Token(base::OpCode::LOAD_LITERAL, literals.push(std::stod(number.str())), pos); // Literal Double
						}
					}

					return {};
				}

				if (!view.isEnd() and (*view == 'u')) {
					view.removePrefix(1);
					return Token(base::OpCode::LOAD_LITERAL, literals.push(std::stoull(number.str())), pos); // Literal Long
				}

				return Token(base::OpCode::LOAD_LITERAL, literals.push(std::stoll(number.str())), pos); // Literal Long
			}

			return {};
		}

		ExtractorResult extractOperator() {
			if (view.isEnd()) {
				return {};
			}

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

			return {};
		}

		ExtractorResult extractOperator(base::OpCode hint) {
			if (view.isEnd()) {
				return {};
			}

			const size_t pos = view.pos();
			const cString opName = opCodeName(hint);
			if (view.startsWith(opName)) {
				view.removePrefix(opName.length);
				return Token(hint, pos);
			}

			return {};
		}

		Token extract() {
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

			throw ex::ParserException("Could not recognize token", view.pos());
		}

		Token extract(base::OpCode hint) {
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
					result = extractOperator(hint);
					break;
				case base::OpCode::IF:
				case base::OpCode::ELSE:
				case base::OpCode::WHILE:
				case base::OpCode::CONTINUE:
				case base::OpCode::BREAK:
				case base::OpCode::FUNC:
				case base::OpCode::RETURN:
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
				return Token(base::OpCode::ERR, view.pos());
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

		Token nextToken(base::OpCode hint) {
			if (!peaked.has_value()) {
				return extract(hint);
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
		base::LiteralStore literals;

		Tokenizer(const std::string& source) :
			view(source) {
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

		Token next(const std::string& message, base::OpCode hint) {
			Token token = nextToken(hint);
			if (token.opCode == base::OpCode::ERR) {
				throw ex::ParserException(message, token.pos);
			}
			return token;
		}

		Token next(base::OpCode hint) {
			Token token = nextToken(hint);
			if (token.opCode == base::OpCode::ERR) {
				throw ex::ParserException("Expected: "s + base::opCodeName(hint).str, token.pos);
			}
			return token;
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
			return next("Expected any of: " + base::opCodeName(expectedOpCode...), expectedOpCode...);
		}
	};
}