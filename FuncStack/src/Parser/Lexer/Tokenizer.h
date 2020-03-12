#pragma once

#include <string>
#include <cctype>
#include <functional>
#include <optional>
#include <sstream>

#include "src/Base/Type.h"
#include "src/Base/OperatorAttributes.h"
#include "src/Base/OpCode.h"

namespace {
	using stringConstIterator = std::string::const_iterator;
	using ExtractorResult = std::optional<base::OpCode>;

	class SourceStream {
		const std::string& source;
		stringConstIterator pos;
		stringConstIterator saved_pos;
	public:
		SourceStream(const std::string& source) : source(source) { pos = this->source.begin(); }
		bool isEnd() const { return pos == source.end(); }
		bool is(char c) const { return !isEnd() and (*pos == c); }
		char peak() const { return *pos; }
		char pop() { return *pos++; }
		void save() { saved_pos = pos; }
		void rewind() { pos = saved_pos; }
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
	};

	class Extractor_Operator : public Extractor {
	public:
		ExtractorResult extract(SourceStream& stream) const override {
			if (!stream.isEnd()) {
				stream.save();
				const int maxSize = base::maxSize();

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
					return base::fromSymbol(validated.c_str()).op;
				}
			}
			return {};
		}
	};

	class Extractor_Name : public Extractor {
	public:
		ExtractorResult extract(SourceStream& stream) const override {
			if (!stream.isEnd()) {
				auto recognizer = [](char c) {return std::isalpha(c) or (c == '_'); };
				if (recognizer(stream.peak())) {
					const std::string lexem = extractLexem(stream, recognizer);

					if ((lexem == "true") or (lexem == "false")) {
						return base::OpCode(base::Operator::LOAD, lexem == "true"); // Literal Bool
					} else if (base::isSymbol(lexem.c_str())) {
						return base::fromSymbol(lexem.c_str()).op; // Type Keyword
					} else {
						return base::OpCode(base::Operator::LOAD, lexem); // Name
					}
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
						return base::OpCode(base::Operator::LOAD, std::stold(currentToken)); // Literal Double
					}
					stream.rewind();
				}
				return base::OpCode(base::Operator::LOAD, std::stoll(currentToken)); // Literal Long
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
	std::vector<base::OpCode> tokenize(const std::string& expression) {
		std::vector<base::OpCode> out;
		SourceStream stream(expression);

		static const std::initializer_list<Extractor*> extractors = {
			new Extractor_Operator(),
			new Extractor_NumberLiteral(),
			new Extractor_Name(),
		};

		while (!stream.isEnd()) {
			if (std::isspace(stream.peak())) {
				stream.pop();
				continue;
			}

			for (const auto& i : extractors) {
				auto result = i->extract(stream);
				if (result.has_value()) {
					out.push_back(result.value());
					continue;
				}
			}
		}

		return out;
	}
}