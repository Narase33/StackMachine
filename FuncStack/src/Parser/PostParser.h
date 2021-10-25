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
				switch (ops.bytecode[currentPos].getOpCode()) {
					case base::OpCode::LABEL:
					{
						const size_t labelId = ops.bytecode[currentPos].unsignedData();

						for (size_t prevJump : jumps[labelId]) {
							base::Operation& prevjumpOp = ops.bytecode[prevJump];

							const int64_t jumpDistance = static_cast<int64_t>(currentPos - prevJump);
							if (std::abs(jumpDistance) > 1) {
								const base::OpCode relativeJump = (prevjumpOp.getOpCode() == base::OpCode::JUMP_LABEL) ? base::OpCode::JUMP : base::OpCode::JUMP_IF_NOT;
								prevjumpOp = base::Operation(relativeJump, jumpDistance);
								optimizedJumps.push_back(prevJump);
							} else {
								deleteFrame(prevJump);
							}
						}

						labels[labelId] = currentPos;
						deleteFrame(currentPos);
					}
					break;
					case base::OpCode::JUMP_LABEL: // fallthrough
					case base::OpCode::JUMP_LABEL_IF_NOT:
					{
						const size_t labelId = ops.bytecode[currentPos].unsignedData();

						const auto jumpDestination = labels.find(labelId);
						if (jumpDestination != labels.end()) {
							base::Operation& jumpDestinationOp = ops.bytecode[currentPos];

							const int64_t jumpDistance = static_cast<int64_t>(jumpDestination->second - currentPos);
							if (std::abs(jumpDistance) > 1) {
								const base::OpCode relativeJump = (jumpDestinationOp.getOpCode() == base::OpCode::JUMP_LABEL) ? base::OpCode::JUMP : base::OpCode::JUMP_IF_NOT;
								jumpDestinationOp = base::Operation(relativeJump, jumpDistance);
								optimizedJumps.push_back(currentPos);
							} else {
								deleteFrame(currentPos);
							}
						} else {
							jumps[labelId].push_back(currentPos);
						}
					}
					break;
				}
				currentPos++;
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