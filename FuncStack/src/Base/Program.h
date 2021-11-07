#pragma once

#include <vector>

#include "LiteralStore.h"

namespace base {
	struct Program {
		Bytecode bytecode;
		base::LiteralStore literals;

		void spliceBytecode(std::vector<Operation> toSplice) {
			bytecode.insert(bytecode.end(), toSplice.begin(), toSplice.end());
		}
	};
}