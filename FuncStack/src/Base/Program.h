#pragma once

#include <vector>

#include "Operation.h"

namespace base {
	struct Program {
		Bytecode bytecode;
		std::vector<BasicType> constants;

		size_t addConstant(BasicType value) {
			const auto pos = std::find_if(constants.begin(), constants.end(), [&](const BasicType& b) {
				return (b == value).getBool();
			});

			const size_t index = std::distance(constants.begin(), pos);
			if (pos == constants.end()) {
				constants.push_back(std::move(value));
			}

			return index;
		}

		const BasicType& getConstant(size_t index) const {
			assert(constants.size() > index);
			return constants[index];
		}

		void spliceBytecode(std::vector<Operation> toSplice) {
			bytecode.insert(bytecode.end(), toSplice.begin(), toSplice.end());
		}
	};
}