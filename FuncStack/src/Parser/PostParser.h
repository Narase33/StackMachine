#pragma once

#include "src/Base/Program.h"

namespace compiler {
	class PostParser {
		using list = std::vector<base::Operation>;

	public:
		PostParser(base::Program& _ops)
			: ops(_ops) {
		}

		void run() {
			while (currentPos < ops.bytecode.size()) {
				
			}
		}

	private:
		base::Program& ops;

		std::unordered_map<size_t, size_t> labels; /* index - position */
		std::unordered_map<size_t, std::vector<size_t>> jumps; /* index - position */

		std::vector<size_t> optimizedJumps;

		size_t currentPos = 0;

		void deleteFrame(size_t pos) {
			ops.bytecode.erase(ops.bytecode.begin() + pos);
			currentPos--;

			for (size_t jump : optimizedJumps) {
				if (jumpsOver(jump, pos)) {
					int32_t& jumpDistance = ops.bytecode[jump].signedData();
					jumpDistance += (jumpDistance > 0) ? -1 : +1;
					// TODO recursive
				}
			}
		}

		bool jumpsOver(size_t jump, size_t pos) const {
			const size_t to = jump + ops.bytecode[jump].signedData();
			auto [min, max] = std::minmax(jump, to);
			return (min < pos) && (pos < max);
		}
	};
}