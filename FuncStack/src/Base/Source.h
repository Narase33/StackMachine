#pragma once

#include <string>
#include <string_view>

namespace base {
	class Source {
	public:
		Source(std::string&& code) :
			code(std::move(code)) {
		}

		Source(const Source&) = delete;
		Source& operator=(const Source&) = delete;

		Source(Source&& other) = default;
		Source& operator=(Source&& other) = default;

		std::string::const_iterator begin() const {
			return code.begin();
		}

		std::string::const_iterator end() const {
			return code.end();
		}

		std::string markedLineAt(std::string::const_iterator pos) const {
			return markedLineAt(std::distance(code.begin(), pos));
		}

		std::string markedLineAt(size_t pos) const {
			size_t lineBegin = pos - 1;
			while ((lineBegin > 0) and (code[lineBegin] != '\n')) {
				lineBegin--;
			}

			size_t lineEnd = pos;
			while ((lineEnd < code.length()) and (code[lineEnd] != '\n')) {
				lineEnd++;
			}

			std::string out = code.substr(lineBegin, lineEnd - lineBegin);
			out += "\n" + std::string(pos - lineBegin - 1, '-') + "^";
			return out;
		}

		const std::string& str() const {
			return code;
		}

	private:
		std::string code;
	};
}