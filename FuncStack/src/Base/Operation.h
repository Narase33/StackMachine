#pragma once

#include "OpCode.h"
#include "StackFrame.h"

namespace base {
	struct Operation final {
		explicit Operation(OpCode opCode, StackFrame value) :
			_opCode(opCode), _value1(std::move(value)) {
		}

		explicit Operation(OpCode opCode) :
			_opCode(opCode), _value1("") {
		}

		explicit Operation() :
			_opCode(OpCode::ERR), _value1("") {
		}

		const StackFrame& value() const {
			return _value1;
		}

		StackFrame& value() {
			return _value1;
		}

		OpCode getOpCode() const {
			return _opCode;
		}

	private:
		OpCode _opCode;
		StackFrame _value1;
	};
}