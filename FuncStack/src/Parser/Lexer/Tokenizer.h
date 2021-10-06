#pragma once

#include <string>
#include <cctype>
#include <functional>
#include <optional>
#include <sstream>

#include "src/Base/StackFrame.h"
#include "src/Base/OperatorAttributes.h"
#include "src/Base/Operation.h"

namespace {
	using stringConstIterator = std::string::const_iterator;
	using ExtractorResult = std::optional<base::Operation>;

	class SourceStream {
		const std::string& source;
		stringConstIterator _pos;
		stringConstIterator _saved_pos;

	public:
		SourceStream(const std::string& source) : source(source) {
			_pos = this->source.begin();
		}

		bool isEnd() const {
			return _pos == source.end();
		}

		bool is(char c) const {
			return !isEnd() and (*_pos == c);
		}

		char peak() const {
			return *_pos;
		}

		char pop() {
			return *_pos++;
		}

		void save() {
			_saved_pos = _pos;
		}

		stringConstIterator pos() {
			return _pos;
		}

		void rewind() {
			rewind(_saved_pos);
		}

		void rewind(stringConstIterator pos) {
			_pos = pos;
		}

		void skipWhiteSpace() {
			while (!isEnd() and std::isspace(*_pos)) {
				pop();
			}
		}
	};

	class Extractor {
	public:
		virtual ExtractorResult extract(SourceStream& stream) const = 0;

	protected:
		std::string extractLexem(SourceStream& stream, std::function<bool(char)> recognizer) const {
			std::string currentToken;
			while (!stream.isEnd() && recognizer(stream.peak())) {
				currentToken += stream.pop();
			}
			return currentToken;
		}

		std::string extractUntilWhitespace(SourceStream& stream) const {
			return extractLexem(stream, [](char c) {
				return !std::isspace(c);
			});
		}

		const std::function<bool(char c)> variableNameRecognizer = [](char c) -> bool {
			return std::isalpha(c) or (c == '_');
		};
	};

	class Extractor_Operator : public Extractor {
	public:
		ExtractorResult extract(SourceStream& stream) const override {
			if (!stream.isEnd()) {
				stream.save();
				const size_t maxSize = base::maxSize();

				std::string op;
				op.reserve(maxSize);
				std::string validated;

				for (int i = 0; i < maxSize and !stream.isEnd(); i++) {
					op += stream.pop();

					if (base::isSymbol(op.c_str())) {
						validated = op;
						stream.save();
					}
				}

				stream.rewind();
				if (!validated.empty()) {
					return base::Operation(base::fromSymbol(validated.c_str()).op);
				}
			}
			return {};
		}
	};

	class Extractor_Name : public Extractor {
	public:
		ExtractorResult extract(SourceStream& stream) const override {
			if (!stream.isEnd()) {
				if (variableNameRecognizer(stream.peak())) {
					const std::string lexem = extractLexem(stream, variableNameRecognizer);

					if ((lexem == "true") or (lexem == "false")) {
						return base::Operation(base::Operator::LOAD, base::StackFrame(base::BasicType(lexem == "true"))); // Literal Bool
					}

					if (base::isSymbol(lexem.c_str())) {
						return base::Operation(base::fromSymbol(lexem.c_str()).op); // Type Keyword
					}

					const size_t variableTypeId = base::BasicType::stringToId(lexem);
					if (variableTypeId != -1) {
						base::BasicType type = BasicType::idToType(variableTypeId);
						return base::Operation(base::Operator::CREATE, base::StackFrame(std::move(type)));
					}

					return base::Operation(base::Operator::LOAD, base::StackFrame(lexem)); // Name
				}
			}

			return {};
		}
	};

	class Extractor_NumberLiteral : public Extractor {
	public:
		ExtractorResult extract(SourceStream& stream) const override {
			if (isDigit(stream)) {
				std::string currentToken = extractNumber(stream);

				if (stream.is('.')) {
					stream.save();
					stream.pop();

					if (isDigit(stream)) {
						currentToken += '.' + extractNumber(stream);
						base::BasicType value(std::stold(currentToken));
						return base::Operation(base::Operator::LOAD, base::StackFrame(std::move(value))); // Literal Double
					}
					stream.rewind();
				}
				return base::Operation(base::Operator::LOAD, base::StackFrame(base::BasicType(std::stoll(currentToken)))); // Literal Long
			}
			return {};
		}

		std::string extractNumber(SourceStream& stream) const {
			std::string currentToken;

			while (isDigit(stream)) {
				currentToken += stream.pop();
			}
			return currentToken;
		}

		bool isDigit(const SourceStream& stream) const {
			return !stream.isEnd() && std::isdigit(stream.peak());
		}
	};
}

namespace lexer {
	class Tokenizer {
		void runExtractors(SourceStream& stream) {
			static const std::initializer_list<Extractor*> extractors = {
				new Extractor_Operator(),
				new Extractor_NumberLiteral(),
				new Extractor_Name(),
			};

			for (Extractor* i : extractors) {
				const std::optional<base::Operation> result = i->extract(stream);
				if (result.has_value()) {
					tokens.push_back(result.value());
					return;
				}
			}
			throw ex::Exception("Unknown token: ");
		}

		Tokenizer(const std::string& expression) {
			SourceStream stream(expression);

			while (!stream.isEnd()) {
				stream.skipWhiteSpace();

				if (stream.isEnd()) {
					return;
				}

				runExtractors(stream);
			}
		}

		std::vector<base::Operation> tokens;

	public:
		static std::vector<base::Operation> run(const std::string& expression) {
			return std::move(Tokenizer(expression).tokens);
		}
	};
}