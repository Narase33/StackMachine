#pragma once

#include "OpCode.h"
#include "BasicType.h"

namespace base {
	struct Operation final {
		explicit Operation(OpCode opCode, int32_t value) : _opCode(opCode) {
			as.signedData = value;
		}

		explicit Operation(OpCode opCode, int64_t value) : _opCode(opCode) {
			as.signedData = value;
			assert(as.signedData == value);
		}

		explicit Operation(OpCode opCode, uint32_t value) : _opCode(opCode) {
			as.unsignedData = value;
		}

		explicit Operation(OpCode opCode, uint64_t value) : _opCode(opCode) {
			as.unsignedData = value;
			assert(as.unsignedData == value);
		}

		explicit Operation(OpCode opCode) : _opCode(opCode) {
			as.unsignedData = 0;
		}

		explicit Operation() : _opCode(OpCode::ERR) {
			as.unsignedData = 0;
		}

		int32_t signedData() const {
			return as.signedData;
		}

		int32_t& signedData() {
			return as.signedData;
		}

		uint32_t unsignedData() const {
			return as.unsignedData;
		}

		uint32_t& unsignedData() {
			return as.unsignedData;
		}

		OpCode getOpCode() const {
			return _opCode;
		}

	private:
		OpCode _opCode;
		union {
			int32_t signedData;
			uint32_t unsignedData;
		} as;
	};
}