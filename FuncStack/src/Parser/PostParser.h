#pragma once

#include "src/Base/Operation.h"
#include "src/Base/StackFrame.h"

namespace parser {
	class PostParser {
		using list = std::list<base::Operation>;

		struct JumpPoint {
			list::iterator frame{};
			size_t pos = 0;
		};

	public:
		static void run(list& ops) {
			PostParser{ ops };
		}

	private:
		list& ops;

		std::unordered_map<size_t, size_t> labels; /* index - position */
		std::unordered_map<size_t, std::vector<JumpPoint>> jumps; /* index - jump-op */

		std::vector<JumpPoint> optimizedJumps;

		size_t currentPos = 0;
		list::iterator it;

		PostParser(list& _ops) : ops(_ops) {
			for (it = ops.begin(); it != ops.end(); it++) {
				switch (it->getOpCode()) {
					case OpCode::LABEL:
					{
						const size_t labelId = it->firstValue().getValue().getUint();

						for (JumpPoint& jumpPoint : jumps[labelId]) {
							const OpCode op = jumpPoint.frame->getOpCode();
							const int jumpDistance = static_cast<int>(currentPos - jumpPoint.pos);

							if (std::abs(jumpDistance) > 1) {
								const OpCode relativeJump = (op == OpCode::JUMP_LABEL) ? OpCode::JUMP : OpCode::JUMP_IF;
								*jumpPoint.frame = base::Operation(relativeJump, base::StackFrame(base::BasicType(jumpDistance)));
								optimizedJumps.push_back(jumpPoint);
							} else {
								deleteFrame(jumpPoint.frame, jumpPoint.pos);
							}
						}

						labels[labelId] = currentPos;
						deleteFrame(it, currentPos);
						break;
					}
					case OpCode::JUMP_LABEL: // fallthrough
					case OpCode::JUMP_LABEL_IF:
					{
						const size_t labelId = it->firstValue().getValue().getUint();

						const auto jumpDestination = labels.find(labelId);
						if (jumpDestination != labels.end()) {
							const OpCode op = it->getOpCode();
							const int jumpDistance = static_cast<int>(jumpDestination->second - currentPos);

							if (std::abs(jumpDistance) > 1) {
								const OpCode relativeJump = (op == OpCode::JUMP_LABEL) ? OpCode::JUMP : OpCode::JUMP_IF;
								*it = base::Operation(relativeJump, base::StackFrame(base::BasicType(jumpDistance)));
								optimizedJumps.push_back(JumpPoint{ it, currentPos });
							} else {
								deleteFrame(it, currentPos);
							}
						} else {
							jumps[labelId].push_back(JumpPoint{ it, currentPos });
						}
						break;
					}
				}
				currentPos++;
			}
		}

		void deleteFrame(list::iterator& it, size_t pos) {
			const auto toDelete = it--;
			ops.erase(toDelete);
			currentPos--;

			for (JumpPoint& jump : optimizedJumps) {
				if (jumpsOver(jump, pos)) {
					sm_int& jumpDistance = jump.frame->firstValue().getValue().getInt();
					jumpDistance += (jumpDistance > 0) ? -1 : +1;
					// TODO recursive
				}
			}
		}

		bool jumpsOver(const JumpPoint& jump, size_t pos) const {
			const size_t from = jump.pos;
			const size_t to = jump.pos + jump.frame->firstValue().getValue().getInt();
			auto [min, max] = std::minmax(from, to);
			return (min < pos) && (pos < max);
		}
	};
}