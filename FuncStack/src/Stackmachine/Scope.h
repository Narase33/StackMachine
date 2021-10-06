#pragma once

#include <memory>
#include <map>

#include "src/Base/Operation.h"

class Scope {
public:
	Scope() {
		newScope();
	}

	void newScope() {
		_variables.emplace_back();
	}

	void leaveScope() {
		_variables.pop_back();
	}

	base::BasicType* get(const std::string& name) {
		for (auto it = _variables.rbegin(); it != _variables.rend(); it++) {
			auto _pos = it->find(name);
			if (_pos != it->end()) {
				return &(_pos->second);
			}
		}

		throw ex::Exception("Variable not defined: " + name);
	}

	const base::BasicType* get(const std::string& name) const {
		for (auto it = _variables.rbegin(); it != _variables.rend(); it++) {
			const auto _pos = it->find(name);
			if (_pos != it->end()) {
				return &(_pos->second);
			}
		}

		throw ex::Exception("Variable not defined: " + name);
	}

	void add(const std::string& name, base::BasicType value) {
		auto variableIt = _variables.back().find(name);
		ex::assure(variableIt == _variables.back().end(), "Variable already defined in this scope: " + name);
		_variables.back()[name] = std::move(value);
	}

	void set(const std::string& name, base::BasicType value) {
		base::BasicType* variable = get(name);
		ex::assure(value.typeId() == variable->typeId(), "Variables have different types: " + variable->toString() + " <- " + value.toString());
		*variable = std::move(value);
	}

private:
	std::vector<std::map<std::string, base::BasicType>> _variables;
};