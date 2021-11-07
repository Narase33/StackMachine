#pragma once

#include <vector>
#include <optional>
#include "Token.h"

namespace compiler {
	struct Variable {
		size_t level;
		std::string name;
		size_t type;

		bool operator==(const Variable& other) const {
			return this->name == other.name;
		}
	};

	class VariableContainer {
		using Iterator = std::vector<Variable>::const_iterator;

	public:
		bool add(Variable t) {
			if (has(t)) {
				return false;
			}

			entities.push_back(std::move(t));
			return true;
		}

		bool has(const Variable& other) const {
			return _getIt(other) != entities.end();
		}

		std::optional<size_t> offset(const Variable& other) const {
			const auto pos = _getIt(other);
			if (pos != entities.end()) {
				return std::distance(entities.begin(), pos);
			}
			return {};
		}

		std::optional<size_t> type(const Variable& other) const {
			const auto pos = _getIt(other);
			if (pos != entities.end()) {
				return pos->type;
			}
			return {};
		}

		Iterator begin() const {
			return entities.begin();
		}

		Iterator end() const {
			return entities.end();
		}

		size_t size() const {
			return entities.size();
		}

		std::vector<Variable>& list() {
			return entities;
		}

		const std::vector<Variable>& list() const {
			return entities;
		}

	private:
		std::vector<Variable> entities;

		const Iterator _getIt(const Variable& other) const {
			return std::find_if(entities.begin(), entities.end(), [&](const Variable& t) {
				return t == other;
			});
		}
	};

	class ScopeDict {
		using Iterator = std::vector<Token>::const_iterator;

	public:
		struct Breaker {
			size_t index, breakCount, level, variables;
			Token token;
		};

		ScopeDict() = default;

		void pushScope() {
			scopeLevel++;
		}

		uint32_t popScope() {
			const auto pos = std::find_if(variables.begin(), variables.end(), [=](const Variable& v) {
				return v.level == scopeLevel;
			});
			uint32_t count = std::distance(pos, variables.end());
			variables.list().erase(pos, variables.end());
			scopeLevel--;
			return count;
		}

		void pushBreaker(size_t index, size_t breakCount, Token token) {
			breakers.push_back({ index, breakCount, scopeLevel, variables.size(), token });
		}

		void pushLoop() {
			for (Breaker& b : breakers) {
				b.breakCount++;
			}
		}

		std::vector<Breaker> popLoop() {
			for (Breaker& b : breakers) {
				b.breakCount--;
			}

			const auto pos = std::remove_if(breakers.begin(), breakers.end(), [](const Breaker& b) {
				return b.breakCount == 0;
			});

			std::vector<Breaker> toResolve;
			std::copy(pos, breakers.end(), std::back_inserter(toResolve));
			breakers.erase(pos, breakers.end());

			return toResolve;
		}

		size_t level() const {
			return scopeLevel;
		}

		// ==== Local variables ====

		bool pushVariable(const std::string& variableName, size_t type) {
			if (hasVariable(variableName)) {
				return false;
			}

			if (scopeLevel == 0) {
				return globalVariables.add({ scopeLevel, variableName, type });
			}
			return variables.add({ scopeLevel, variableName, type });
		}

		bool hasVariable(const std::string& variableName) const {
			return variables.has({ 0, variableName, 0 })
				or globalVariables.has({ 0, variableName, 0 });
		}

		size_t sizeLocalVariables() const {
			return variables.size();
		}

		// === Other ====

		base::Operation createStoreOperation(const Token& token) const {
			const std::string& name = std::get<std::string>(token.value);

			std::optional<size_t> variableOffset = variables.offset({ 0, name, 0 });
			if (variableOffset.has_value()) {
				return base::Operation(base::OpCode::STORE_LOCAL, variableOffset.value());
			}

			variableOffset = globalVariables.offset({ 0, name, 0 });
			if (variableOffset.has_value()) {
				return base::Operation(base::OpCode::STORE_GLOBAL, variableOffset.value());
			}

			throw ex::ParserException("used variable not declared", token.pos);
		}

		base::Operation createLoadOperation(const Token& token) const {
			const std::string& name = std::get<std::string>(token.value);

			std::optional<size_t> variableOffset = variables.offset({ 0, name, 0 });
			if (variableOffset.has_value()) {
				return base::Operation(base::OpCode::LOAD_LOCAL, variableOffset.value());
			}

			variableOffset = globalVariables.offset({ 0, name, 0 });
			if (variableOffset.has_value()) {
				return base::Operation(base::OpCode::LOAD_GLOBAL, variableOffset.value());
			}

			throw ex::ParserException("used variable not declared", token.pos);
		}

		size_t typeOf(const Token& token) const {
			const std::string& name = std::get<std::string>(token.value);

			std::optional<size_t> type = variables.type({ 0, name, 0 });
			if (!type.has_value()) {
				type = globalVariables.type({ 0, name , 0 });
			}

			if (!type.has_value()) {
				throw ex::ParserException("used variable not declared", token.pos);
			}

			return type.value();
		}

	private:
		std::vector<Breaker> breakers;
		VariableContainer variables;
		VariableContainer globalVariables;

		size_t scopeLevel = 0;

		void assume(bool condition, const std::string& message, Iterator it) const {
			if (!condition) {
				throw ex::ParserException(message, it->pos);
			}
		}
	};
}