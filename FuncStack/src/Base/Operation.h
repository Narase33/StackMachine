#pragma once

#include "OpCode.h"
#include "StackFrame.h"

namespace base {
	struct Operation final {
		explicit Operation(OpCode opCode, StackFrame value1, StackFrame value2) :
			_opCode(opCode), _value1(std::move(value1)), _value2(std::move(value2)) {
		}

		explicit Operation(OpCode opCode, StackFrame value) :
			_opCode(opCode), _value1(std::move(value)), _value2("") {
		}

		explicit Operation(OpCode opCode) :
			_opCode(opCode), _value1(""), _value2("") {
		}

		explicit Operation() :
			_opCode(OpCode::ERR), _value1(""), _value2("") {
		}

		const StackFrame& firstValue() const {
			return _value1;
		}

		StackFrame& firstValue() {
			return _value1;
		}

		const StackFrame& secondValue() const {
			return _value2;
		}

		StackFrame& secondValue() {
			return _value2;
		}

		OpCode getOpCode() const {
			return _opCode;
		}

	private:
		OpCode _opCode;
		StackFrame _value1;
		StackFrame _value2;
	};
}