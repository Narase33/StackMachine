#pragma once

#include <unordered_map>
#include <tuple>
#include <initializer_list>

#include "src/Utils/StringWindow.h"

namespace base {
	using sm_int = int64_t;
	using sm_uint = uint64_t;
	using sm_float = double;
	using sm_bool = bool;

	enum class TypeIndex {
		Err = -1, Int = 0, Uint = 1, Float = 2, Bool = 3
	};

#define SM_REGISTER_NAME(x) case x: return #x
	std::string idToString(TypeIndex opCode) {
		switch (opCode) {
			SM_REGISTER_NAME(TypeIndex::Err);
			SM_REGISTER_NAME(TypeIndex::Int);
			SM_REGISTER_NAME(TypeIndex::Uint);
			SM_REGISTER_NAME(TypeIndex::Float);
			SM_REGISTER_NAME(TypeIndex::Bool);
		}
		return "";
	}
#undef SM_REGISTER_NAME

	TypeIndex stringToId(const StringWindow& str) {
		if (str == "int") return TypeIndex::Int;
		if (str == "uint") return TypeIndex::Uint;
		if (str == "float") return TypeIndex::Float;
		if (str == "bool") return TypeIndex::Bool;
		return TypeIndex::Err;
	}

	size_t sizeOfType(TypeIndex index) {
		switch (index) {
			case TypeIndex::Int: return sizeof(sm_int);
			case TypeIndex::Uint: return sizeof(sm_uint);
			case TypeIndex::Float: return sizeof(sm_float);
			case TypeIndex::Bool: return sizeof(sm_bool);
			default: return 0;
		}
	}
}