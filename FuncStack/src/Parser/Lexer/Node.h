#pragma once

#include <vector>

#include "src/Base/Operation.h"
#include "src/Utils/Utils.h"
#include "src/Exception.h"

namespace lexer {
	class Node {
		base::Operation op;
		std::vector<Node> group;

	public:
		Node(base::Operation op)
			: op(std::move(op)) {
		}

		Node(base::Operation op, std::vector<Node> group)
			: op(std::move(op)), group(std::move(group)) {
			ex::assure(isGroup(), "Got non-group StackFrame");
		}

		const base::Operation& getOperation() const {
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