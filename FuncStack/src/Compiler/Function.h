#pragma once

#include <vector>
#include <string>

#include "src/Base/AtomicTypes.h"

namespace compiler {
	struct Function {
		struct Variable {
			base::TypeIndex type;
			std::string name;

			bool operator==(const Variable& other) const {
				return (this->type == other.type)
					and (this->name == other.name);
			}
		};

		std::string name;
		std::optional<base::TypeIndex> returnType;
		std::vector<Variable> params;
		size_t bytecodeOffset;

		bool operator==(const Function& other) const {
			return (this->name == other.name)
				and (this->returnType == other.returnType)
				and (this->params == other.params);
		}
	};

	class FunctionCache {
	public:
		bool push(std::string functionName, std::optional<base::TypeIndex> returnType, std::vector<Function::Variable> params, size_t bytecodeOffset) { // TODO optimize
			if (!has(functionName, returnType, params)) {
				functions.push_back({ functionName, returnType, params, bytecodeOffset });
				return true;
			}
			return false;
		}

		std::optional<size_t> offset(const std::string& functionName, std::optional<base::TypeIndex> returnType, const std::vector<Function::Variable>& params) const { // TODO optimize
			const auto pos = std::find(functions.begin(), functions.end(), Function{ functionName, returnType, params, 0 });
			if (pos != functions.end()) {
				return pos->bytecodeOffset;
			}
			return {};
		}

		size_t paramCount(const std::string& functionName) const {
			const auto pos = std::find_if(functions.begin(), functions.end(), [&](const Function& f) {
				return f.name == functionName;
			});
			return pos->params.size();
		}

		bool has(const std::string& functionName, std::optional<base::TypeIndex> returnType, const std::vector<Function::Variable>& params) const { // TODO optimize
			return std::find(functions.begin(), functions.end(), Function{ functionName, returnType, params, 0 }) != functions.end();
		}

		std::optional<size_t> offset(const std::string& functionName) const { // TODO TEMPORARY !!!
			const auto pos = std::find_if(functions.begin(), functions.end(), [&](const Function& f) {
				return f.name == functionName;
			});

			if (pos != functions.end()) {
				return pos->bytecodeOffset;
			}
			return {};
		}

		bool has(const std::string& functionName) const { // TODO TEMPORARY !!!
			return std::find_if(functions.begin(), functions.end(), [&](const Function& f) {
				return f.name == functionName;
			}) != functions.end();
		}

	private:
		std::vector<Function> functions;
	};
}