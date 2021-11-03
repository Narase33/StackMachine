#pragma once

#include <vector>

#include "OpCode.h"
#include "BasicType.h"

namespace base {
	class Operation final {
	public:
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

		int16_t side_signedData() const {
			return side.signedData;
		}

		int16_t& side_signedData() {
			return side.signedData;
		}

		uint16_t side_unsignedData() const {
			return side.unsignedData;
		}

		uint16_t& side_unsignedData() {
			return side.unsignedData;
		}

		OpCode getOpCode() const {
			return _opCode;
		}

	private:
		OpCode _opCode;

		union {
			int16_t signedData;
			uint16_t unsignedData;
		} side;

		union {
			int32_t signedData;
			uint32_t unsignedData;
		} as;
	};

	using Bytecode = std::vector<Operation>;
}