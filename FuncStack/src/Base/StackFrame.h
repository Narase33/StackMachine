#pragma once

#include <sstream>

#include "BasicType.h"
#include "ArrayType.h"

namespace base {
	class StackFrame final {
	public:
		explicit StackFrame() = default;
		explicit StackFrame(BasicType value) : inner(std::move(value)) {}
		explicit StackFrame(std::string name) : inner(std::move(name)) {}

		bool isVariable() const {
			return std::holds_alternative<std::string>(inner);
		}

		bool isValue() const {
			return std::holds_alternative<base::BasicType>(inner);
		}

		const std::string& getName() const {
			return std::get<std::string>(inner);
		}

		const BasicType& getValue() const {
			return std::get<base::BasicType>(inner);
		}

		BasicType& getValue() {
			return std::get<base::BasicType>(inner);
		}

		friend bool operator==(const StackFrame& a, const StackFrame& b) {
			if (a.isValue() != b.isValue()) {
				return false;
			}

			if (a.isValue()) {
				return (a.getValue() == b.getValue()).getBool();
			} else {
				return a.getName() == b.getName();
			}
		}

	private:
		std::variant<BasicType, std::string> inner;
	};

	std::string printStack(const std::list<base::StackFrame>& s) {
		using namespace base;
		std::ostringstream stream;

		stream << "Stack:" << std::endl;
		for (const auto& i : s) {
			if (i.isValue()) {
				stream << "\t" << i.getValue() << std::endl;
			} else {
				stream << "\t" << i.getName() << std::endl;
			}
		}

		return stream.str();
	}

} // namespace base