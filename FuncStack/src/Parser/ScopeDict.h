#pragma once

#include "Token.h"

namespace compiler {
	class ScopeDict {
		using Iterator = std::vector<Token>::const_iterator;

	public:
		struct LoopLayer {
			size_t level, head, end;
		};

		struct Variable {
			size_t level;
			std::string name;
		};

		ScopeDict() = default;

		void pushScope() {
			scopeLevel++;
		}

		void popScope() {
			const auto pos = std::find_if(variables.begin(), variables.end(), [=](const Variable& v) {
				return v.level == scopeLevel;
			});
			variables.erase(pos, variables.end());
			scopeLevel--;
		}

		void pushLoop(size_t headLabel, size_t endLabel) {
			loops.push_back({ scopeLevel, headLabel, endLabel });
		}

		void popLoop() {
			loops.pop_back();
		}

		bool pushVariable(const std::string& variableName) {
			if (hasVariable(variableName)) {
				return false;
			}

			variables.push_back({ scopeLevel, variableName });
			return true;
		}

		std::optional<size_t> offsetVariable(const std::string& variableName) const {
			const auto pos = _getVariableIt(variableName);
			if (pos != variables.end()) {
				return std::distance(variables.begin(), pos);
			}
			return {};
		}

		bool hasVariable(const std::string& variableName) const {
			return _getVariableIt(variableName) != variables.end();
		}

		std::optional<LoopLayer> loopLabelFromOffset(size_t offset) const {
			if (loops.size() < offset) {
				return {};
			}

			return loops[loops.size() - 1 - offset];
		}

		size_t level() const {
			return scopeLevel;
		}

		base::Operation createStoreOperation(Iterator it) const {
			const std::optional<size_t> variableOffset = offsetVariable(std::get<std::string>(it->value));
			assume(variableOffset.has_value(), "Variable not declared", it);
			return base::Operation(base::OpCode::STORE, base::BasicType(variableOffset.value()));
		}

		base::Operation createLoadOperation(Iterator it) const {
			const std::optional<size_t> variableOffset = offsetVariable(std::get<std::string>(it->value));
			assume(variableOffset.has_value(), "used variable not declared", it);
			return base::Operation(base::OpCode::LOAD, base::BasicType(variableOffset.value()));
		}

	private:
		std::vector<LoopLayer> loops;
		std::vector<Variable> variables;
		size_t scopeLevel = 0;

		const std::vector<Variable>::const_iterator _getVariableIt(const std::string& name) const {
			return std::find_if(variables.begin(), variables.end(), [&](const Variable& v) {
				return v.name == name;
			});
		}

		void assume(bool condition, const std::string& message, Iterator it) const {
			if (!condition) {
				throw ex::ParserException(message, it->pos);
			}
		}
	};
}