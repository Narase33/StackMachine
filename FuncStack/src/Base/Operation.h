#pragma once

#include "OperatorAttributes.h"

namespace base {
	struct Operation {
		explicit Operation(Operator opCode, StackFrame value_a, StackFrame value_b) :
			_opCode(opCode), _value{ std::move(value_a), std::move(value_b) } {
		}

		explicit Operation(Operator opCode, StackFrame value) :
			_opCode(opCode), _value{ std::move(value), std::nullopt } {
		}

		explicit Operation(Operator opCode)
			: _opCode(opCode), _value{ std::nullopt, std::nullopt } {
		}

		explicit Operation()
			: _opCode(Operator::ERR), _value{ std::nullopt, std::nullopt } {
		}

		const StackFrame& getStackFrame(size_t index) const {
			return _value[index].value();
		}

		StackFrame& getStackFrame(size_t index) {
			return _value[index].value();
		}

		void setStackFrame(size_t index, StackFrame value) {
			_value[index] = value;
		}

		bool hasStackFrame(size_t index) const {
			return _value[index].has_value();
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

		bool operator<(const Operation& other) const {
			return getPriority(_opCode) < getPriority(other.getOperator());
		}

		bool operator>(const Operation& other) const {
			return getPriority(_opCode) > getPriority(other.getOperator());
		}

		bool operator==(const Operation& other) const {
			return (getOperator() == other.getOperator()) and (getStackFrame(0) == other.getStackFrame(0) and (getStackFrame(1) == other.getStackFrame(1)));
		}

	private:
		Operator _opCode;
		std::optional<StackFrame> _value[2];
	};
}