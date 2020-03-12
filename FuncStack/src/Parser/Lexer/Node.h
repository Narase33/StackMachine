#pragma once

#include <vector>

#include "src/Base/OpCode.h"
#include "src/Utils/Utils.h"
#include "src/Exception.h"

namespace lexer {
	class Node {
		base::OpCode op;
		std::vector<Node> group;

	public:
		Node(base::OpCode op) : op(op) {
			// empty 
		}

		Node(base::OpCode op, std::vector<Node> group) : op(op), group(std::move(group)) {
			ex::assure(isGroup(), "Got non-group OpCode");
		}

		base::OpCode getOpCode() const {
			return op;
		}

		base::Operator getOperator() const {
			return op.getOperator();
		}

		bool isGroup() const {
			return utils::any_of(op.getOperator(), { base::Operator::BRACE_GROUP, base::Operator::BRACKET_GROUP });
		}

		bool is(base::Operator op) const {
			return this->op.isOperator(op);
		}

		std::vector<Node> getGroup() const {
			return group;
		}
	};
}