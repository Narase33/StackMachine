#pragma once

#include <memory>
#include <map>

#include "src/Base/StackFrame.h"

class Scope {
public:
	Scope(Scope* parent) : _parent(parent) {
		// empty
	}

	std::optional<base::ValueType> get(const std::string& name) const {
		const auto pos = _variables.find(name);
		if (pos != _variables.end()) {
			return pos->second;
		}

		if (_parent != nullptr) {
			return _parent->get(name);
		}

		return {};
	}

	void add(const std::string& name, base::ValueType value) {
		_variables[name] = value;
	}

private:
	std::map<std::string, base::ValueType> _variables;
	Scope* _parent;
};