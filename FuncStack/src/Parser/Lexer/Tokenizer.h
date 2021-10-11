#pragma once

#include <string>
#include <cctype>
#include <functional>
#include <optional>
#include <sstream>

#include "Token.h"

namespace {
	using Iterator = std::string::const_iterator;
	using ExtractorResult = std::optional<Token>;

	void skipWhitespace(Iterator& current, Iterator end) {
		while ((current != end) and std::isspace(*current)) {
			current++;
		}
	}

	bool partOfVariableName(char c) {
		return std::isalpha(c) or (c == '_');
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
					const OpCode symbol = opCodeFromSymbol(op);
					if (symbol != OpCode::ERR) {
						current += 2;
						return Token(symbol, pos());
					}
				}

				const std::string_view op(current, current + 1);
				const OpCode symbol = opCodeFromSymbol(op);
				if (symbol != OpCode::ERR) {
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
					return Token(OpCode::LITERAL, lexem == "true", pos()); // Literal Bool
				}

				const OpCode symbol = opCodeFromKeyword(lexem);
				if (symbol != OpCode::ERR) {
					return Token(symbol, pos()); // Type Keyword
				}

				const size_t variableTypeId = base::BasicType::stringToId(lexem);
				if (variableTypeId != -1) {
					return Token(OpCode::TYPE, variableTypeId, pos());
				}

				return Token(OpCode::NAME, lexem, pos()); // Name
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
							return Token(OpCode::LITERAL, std::stod(beforeDot + "." + afterDot), pos()); // Literal Double
						}
					}
					current = save;
				}

				if (!isEnd() and (*current == 'u')) {
					current++;
					return Token(OpCode::LITERAL, static_cast<sm_uint>(std::stoul(beforeDot)), pos()); // Literal Long
				}
				return Token(OpCode::LITERAL, static_cast<sm_int>(std::stol(beforeDot)), pos()); // Literal Long
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
}

namespace lexer {
	class Tokenizer {
		std::vector<Token> tokens;
		bool success = true;

		Iterator current;
		const Iterator begin;
		const Iterator end;
		const Source& source;

		void runToNextSync() {
			while ((current != end) and (!std::isspace(*current))) {
				current++;
			}
		}

		void runExtractors(std::initializer_list<Extractor*> extractors) {
			for (Extractor* i : extractors) {
				ExtractorResult result = i->extract();
				if (result.has_value()) {
					tokens.push_back(std::move(result.value()));
					return;
				}
			}
			throw ex::ParserException("Unknown token", std::distance(begin, current));
		}

	public:
		Tokenizer(const Source& source) :
			source(source), begin(source.begin()), current(source.begin()), end(source.end()) {
		}

		bool isSuccess() const {
			return success;
		}

		std::vector<Token> run() {
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
					runExtractors(extractors);
				} catch (const ex::ParserException& ex) {
					std::cout << ex.what() << "\n" << source.markedLineAt(ex.getPos()) << std::endl;
					runToNextSync();
					success = false;
				}
			}

			return std::move(tokens);
		}
	};
}