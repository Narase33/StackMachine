#pragma once

#include "Token.h"

namespace compiler {
	template <typename T>
	class EntityContainer {
		using Iterator = std::vector<T>::const_iterator;

	public:
		bool add(T t) {
			if (has(t)) {
				return false;
			}

			entities.push_back(std::move(t));
			return true;
		}

		bool has(const T& other) const {
			return _getIt(other) != entities.end();
		}

		std::optional<size_t> offset(const T& other) const {
			const auto pos = _getIt(other);
			if (pos != entities.end()) {
				return std::distance(entities.begin(), pos);
			}
			return {};
		}

		Iterator begin() const {
			return entities.begin();
		}

		Iterator end() const {
			return entities.end();
		}

		std::vector<T>& list() {
			return entities;
		}

		const std::vector<T>& list() const {
			return entities;
		}

	private:
		std::vector<T> entities;

		const Iterator _getIt(const T& other) const {
			return std::find_if(entities.begin(), entities.end(), [&](const T& t) {
				return t == other;
			});
		}
	};

	class ScopeDict {
		using Iterator = std::vector<Token>::const_iterator;

	public:
		struct Breaker {
			size_t index, breakCount, level;
			Token token;
		};

		struct Variable {
			size_t level;
			std::string name;
			size_t type;

			bool operator==(const Variable& other) const {
				return this->name == other.name;
			}
		};

		ScopeDict() = default;

		void pushScope() {
			scopeLevel++;
		}

		void popScope() {
			const auto pos = std::find_if(variables.begin(), variables.end(), [=](const Variable& v) {
				return v.level == scopeLevel;
			});
			variables.list().erase(pos, variables.end());
			scopeLevel--;
		}

		void pushBreaker(size_t index, size_t breakCount, Token token) {
			breakers.push_back({ index, breakCount, scopeLevel, token });
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

		std::optional<size_t> offsetLocalVariable(const std::string& variableName) const {
			return variables.offset({ 0, variableName, 0 });
		}

		std::optional<size_t> offsetGlobalVariable(const std::string& variableName) const {
			return globalVariables.offset({ 0, variableName, 0 });
		}

		// === Other ====

		base::Operation createStoreOperation(const Token& token) const {
			const std::string& name = std::get<std::string>(token.value);

			std::optional<size_t> variableOffset = offsetLocalVariable(name);
			if (variableOffset.has_value()) {
				return base::Operation(base::OpCode::STORE_LOCAL, variableOffset.value());
			}

			variableOffset = offsetGlobalVariable(name);
			if (variableOffset.has_value()) {
				return base::Operation(base::OpCode::STORE_GLOBAL, variableOffset.value());
			}

			throw ex::ParserException("used variable not declared", token.pos);
		}

		base::Operation createLoadOperation(const Token& token) const {
			const std::string& name = std::get<std::string>(token.value);

			std::optional<size_t> variableOffset = offsetLocalVariable(name);
			if (variableOffset.has_value()) {
				return base::Operation(base::OpCode::LOAD_LOCAL, variableOffset.value());
			}

			variableOffset = offsetGlobalVariable(name);
			if (variableOffset.has_value()) {
				return base::Operation(base::OpCode::LOAD_GLOBAL, variableOffset.value());
			}

			throw ex::ParserException("used variable not declared", token.pos);
		}

	private:
		std::vector<Breaker> breakers;
		EntityContainer<Variable> variables;
		EntityContainer<Variable> globalVariables;

		size_t scopeLevel = 0;

		void assume(bool condition, const std::string& message, Iterator it) const {
			if (!condition) {
				throw ex::ParserException(message, it->pos);
			}
		}
	};
}