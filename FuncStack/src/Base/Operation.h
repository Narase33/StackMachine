#pragma once

#include "OpCode.h"
#include "BasicType.h"

namespace base {
	struct Operation final {
		explicit Operation(OpCode opCode, BasicType value) :
			_opCode(opCode), _value(std::move(value)) {
		}

		explicit Operation(OpCode opCode) :
			_opCode(opCode) {
		}

		explicit Operation() :
			_opCode(OpCode::ERR) {
		}

		const BasicType& value() const {
			return _value;
		}

		BasicType& value() {
			return _value;
		}

		OpCode getOpCode() const {
			return _opCode;
		}

	private:
		OpCode _opCode;
		BasicType _value;
	};
}