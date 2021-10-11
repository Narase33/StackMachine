#pragma once

#include <vector>

#include "src/Base/Operation.h"
#include "src/Utils/Utils.h"
#include "src/Exception.h"
#include "Token.h"

namespace lexer {
	class Node {
		Token token;
		std::vector<Node> group;

	public:
		Node(Token op)
			: token(std::move(op)) {
		}

		Node(Token op, std::vector<Node> group)
			: token(std::move(op)), group(std::move(group)) {
			ex::assume(isGroup(), "Got non-group Token");
		}

		const Token& getToken() const {
			return token;
		}

		OpCode getOpCode() const {
			return token.id;
		}

		bool isGroup() const {
			return utils::any_of(token.id, { OpCode::BRACKET_ROUND_OPEN, OpCode::BRACKET_CURLY_OPEN });
		}

		bool is(OpCode id) const {
			return token.id == id;
		}

		const std::vector<Node>& getGroup() const {
			return group;
		}
	};
}