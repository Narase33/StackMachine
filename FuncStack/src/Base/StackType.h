#pragma once

#include <sstream>

#include "ValueType.h"

namespace base {
	class StackType {
	public:
		explicit StackType() = default;
		explicit StackType(ValueType valueType) : inner(std::move(valueType)) {}
		explicit StackType(std::string name) : inner(std::move(name)) {}

		bool isVariable() const {
			return std::holds_alternative<std::string>(inner);
		}

		bool isValue() const {
			return std::holds_alternative<base::ValueType>(inner);
		}

		std::string getVariableName() const {
			return std::get<std::string>(inner);
		}

		const ValueType& getValue() const {
			return std::get<base::ValueType>(inner);
		}

		ValueType& getValue() {
			return std::get<base::ValueType>(inner);
		}

		friend bool operator==(const StackType& a, const StackType& b) {
			if (a.isValue() != b.isValue()) {
				return false;
			}

			if (a.isValue()) {
				return (a.getValue() == b.getValue()).getBool();
			} else {
				return a.getVariableName() == b.getVariableName();
			}
		}

	private:
		std::variant<ValueType, std::string> inner;
	};

	std::string printStack(const std::list<base::StackType>& s) {
		using namespace base;
		std::ostringstream stream;

		stream << "Stack:" << std::endl;
		for (const auto& i : s) {
			if (i.isValue()) {
				stream << "\t" << i.getValue() << std::endl;
			} else {
				stream << "\t" << i.getVariableName() << std::endl;
			}
		}

		return stream.str();
	}

} // namespace base