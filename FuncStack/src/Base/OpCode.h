#pragma once

#include "OperatorAttributes.h"

namespace base {
	struct OpCode {
		OpCode(Operator opCode, StackType value) : _opCode(opCode), _value(value) { /* empty */ }
		OpCode(Operator opCode) : _opCode(opCode), _value(std::nullopt) { /* empty */ }
		OpCode() : OpCode(Operator::ERR) { /* empty */ }

		StackType value() const { return _value.value(); }
		void setValue(StackType value) { _value = value; }
		bool hasValue() const { return _value.has_value(); }
		Operator getOperator() const { return _opCode; }
		bool isOperator(Operator op) const { return getOperator() == op; }

		bool operator<(const OpCode& other) const { return getPriority(_opCode) < getPriority(other.getOperator()); }
		bool operator>(const OpCode& other) const { return getPriority(_opCode) > getPriority(other.getOperator()); }
		bool operator==(const OpCode& other) const { return getOperator() == other.getOperator() and value() == other.value(); }

	private:
		Operator _opCode;
		std::optional<StackType> _value;
	};
}