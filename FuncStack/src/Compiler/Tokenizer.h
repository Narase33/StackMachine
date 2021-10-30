#pragma once

#include <string>
#include <cctype>
#include <functional>
#include <optional>
#include <sstream>

#include "Token.h"
#include "src/Base/Source.h"

namespace compiler {
	using Iterator = std::string::const_iterator;
	using ExtractorResult = std::optional<Token>;

	inline void skipWhitespace(Iterator& current, Iterator end) {
		while ((current != end) and std::isspace(*current)) {
			current++;
		}
	}

	inline bool partOfVariableName(char c) {
		return std::isalnum(c) or (c == '_');
	};

	class Extractor {
	public:
		Extractor(Iterator& begin, Iterator end) :
			begin(begin), current(begin), end(end) {
		}

		virtual ExtractorResult extract() const = 0;

	protected:
		Iterator& current;
		const Iterator end;
		const Iterator begin;

		std::string extractLexem(std::function<bool(char)> recognizer) const {
			std::string currentToken;
			while (!isEnd() && recognizer(*current)) {
				currentToken += *current++;
			}
			return currentToken;
		}

		std::string extractUntilWhitespace() const {
			return extractLexem([](char c) {
				return !std::isspace(c);
			});
		}

		bool isEnd() const {
			return current == end;
		}

		size_t pos() const {
			return std::distance(begin, current);
		}
	};

	class Extractor_Operator : public Extractor {
	public:
		using Extractor::Extractor;

		ExtractorResult extract() const override {
			const Iterator save = current;

			if (!isEnd()) {
				if ((current + 1) != end) {
					const std::string_view op(current, current + 2);
					const base::OpCode symbol = base::opCodeFromSymbol(op);
					if (symbol != base::OpCode::ERR) {
						current += 2;
						return Token(symbol, pos());
					}
				}

				const std::string_view op(current, current + 1);
				const base::OpCode symbol = base::opCodeFromSymbol(op);
				if (symbol != base::OpCode::ERR) {
					current += 1;
					return Token(symbol, pos());
				}
			}

			current = save;
			return {};
		}
	};

	class Extractor_Name : public Extractor {
	public:
		using Extractor::Extractor;

		ExtractorResult extract() const override {
			const Iterator save = current;

			if (!isEnd() and (partOfVariableName(*current))) {
				const std::string lexem = extractLexem(partOfVariableName);

				if ((lexem == "true") or (lexem == "false")) {
					return Token(base::OpCode::LOAD_LITERAL, lexem == "true", pos()); // Literal Bool
				}

				const base::OpCode symbol = base::opCodeFromKeyword(lexem);
				if (symbol != base::OpCode::ERR) {
					return Token(symbol, pos()); // Type Keyword
				}

				const base::TypeIndex variableTypeId = base::stringToId(lexem);
				if (variableTypeId != base::TypeIndex::Err) {
					return Token(base::OpCode::TYPE, static_cast<size_t>(variableTypeId), pos());
				}

				return Token(base::OpCode::NAME, lexem, pos()); // Name
			}

			current = save;
			return {};
		}
	};

	class Extractor_NumberLiteral : public Extractor {
	public:
		using Extractor::Extractor;

		ExtractorResult extract() const override {
			Iterator save = current;

			if (isDigit()) {
				const std::string beforeDot = extractNumber();

				if (!isEnd() and (*current == '.')) {
					save = current++;

					if (isDigit()) {
						const std::string afterDot = extractNumber();
						if (!std::isalpha(*current)) {
							return Token(base::OpCode::LOAD_LITERAL, std::stod(beforeDot + "." + afterDot), pos()); // Literal Double
						}
					}
					current = save;
				}

				if (!isEnd() and (*current == 'u')) {
					current++;
					return Token(base::OpCode::LOAD_LITERAL, static_cast<base::sm_uint>(std::stoul(beforeDot)), pos()); // Literal Long
				}
				return Token(base::OpCode::LOAD_LITERAL, static_cast<base::sm_int>(std::stol(beforeDot)), pos()); // Literal Long
			}

			current = save;
			return {};
		}

		std::string extractNumber() const {
			std::string currentToken;

			while (isDigit()) {
				currentToken += *current++;
			}

			return currentToken;
		}

		bool isDigit() const {
			return !isEnd() and std::isdigit(*current);
		}
	};

	class Tokenizer {
		std::optional<Token> peaked;
		bool success = true;

		Iterator current;
		const Iterator begin;
		const Iterator end;
		const base::Source& source;

		void runToNextSync() {
			while ((current != end) and (!std::isspace(*current))) {
				current++;
			}
		}

		Token runExtractors(std::initializer_list<Extractor*> extractors) {
			for (Extractor* i : extractors) {
				ExtractorResult result = i->extract();
				if (result.has_value()) {
					return result.value();
				}
			}
			throw ex::ParserException("Unknown token", std::distance(begin, current));
		}

		Token extract() {
			std::initializer_list<Extractor*> extractors = {
				new Extractor_Operator(current, end),
				new Extractor_NumberLiteral(current, end),
				new Extractor_Name(current, end),
			};

			while (current != end) {
				skipWhitespace(current, end);

				if (current == end) {
					break;
				}

				try {
					return runExtractors(extractors);
				} catch (const ex::ParserException& ex) {
					std::cout << ex.what() << "\n" << source.markedLineAt(ex.getPos()) << std::endl;
					runToNextSync();
					success = false;
				}
			}

			return Token(base::OpCode::END_PROGRAM, std::distance(begin, end));
		}

	public:
		Tokenizer(const base::Source& source) :
			source(source), begin(source.begin()), current(source.begin()), end(source.end()) {
		}

		bool isSuccess() const {
			return success;
		}

		Token nextToken() {
			if (!peaked.has_value()) {
				return extract();
			}

			Token t = peaked.value();
			peaked.reset();
			return t;
		}

		const Token& peak() {
			if (!peaked.has_value()) {
				peaked = extract();
			}
			return peaked.value();
		}
	};
}