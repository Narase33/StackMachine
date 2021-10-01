#pragma once

#include "OperatorAttributes.h"

namespace base {
	struct StackFrame {
		explicit StackFrame(Operator opCode, StackType value) : _opCode(opCode), _value(value) {}
		explicit StackFrame(Operator opCode) : _opCode(opCode), _value(std::nullopt) {}
		explicit StackFrame() : StackFrame(Operator::ERR) {}

		StackType value() const {
			return _value.value();
		}

		void setValue(StackType value) {
			_value = value;
		}

		bool hasValue() const {
			return _value.has_value();
		}

		Operator getOperator() const {
			return _opCode;
		}

		bool isOperator(Operator op) const {
			return getOperator() == op;
		}

		int priority() const {
			return getPriority(_opCode);
		}

		bool operator<(const StackFrame& other) const {
			return getPriority(_opCode) < getPriority(other.getOperator());
		}

		bool operator>(const StackFrame& other) const {
			return getPriority(_opCode) > getPriority(other.getOperator());
		}

		bool operator==(const StackFrame& other) const {
			return getOperator() == other.getOperator() and value() == other.value();
		}

	private:
		Operator _opCode;
		std::optional<StackType> _value;
	};
}