#pragma once

#include <vector>

#include "Operation.h"

namespace base {
	struct Program {
		std::vector<base::Operation> bytecode;
		std::vector<base::BasicType> constants;

		size_t addConstant(base::BasicType value) {
			const auto pos = std::find_if(constants.begin(), constants.end(), [&](const base::BasicType& b) {
				return (b == value).getBool();
			});

			const size_t index = std::distance(constants.begin(), pos);
			if (pos == constants.end()) {
				constants.push_back(std::move(value));
			}

			return index;
		}

		const base::BasicType& getConstant(size_t index) const {
			assert(constants.size() > index);
			return constants[index];
		}

		void spliceBytecode(std::vector<base::Operation> toSplice) {
			bytecode.insert(bytecode.end(), toSplice.begin(), toSplice.end());
		}
	};
}