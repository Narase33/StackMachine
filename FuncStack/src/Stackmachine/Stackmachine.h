#pragma once

#include <list>
#include <sstream>

#include "src/Base/OpCode.h"
#include "src/Utils/Utils.h"
#include "src/Exception.h"

namespace stackmachine {
	class StackMachine {
	public:
		StackMachine() = default;

		void add(base::OpCode op) {
			if ((op.getOperator() == base::Operator::LOAD) and op.value().holds<std::string>() and (variables.find(op.value().get<std::string>()) == variables.end())) {
				variables[op.value().get<std::string>()] = std::nullopt;
			}

			opStack.push_back(op);
		}

		void add(base::Operator op) {
			opStack.emplace_back(op);
		}

		void add(base::Operator op, base::ValueType data) {
			opStack.emplace_back(op, data);
		}

		void add(base::Operator op, const std::string& data) {
			if ((op == base::Operator::LOAD) && (variables.find(data) == variables.end())) {
				variables[data] = std::nullopt;
			}

			opStack.emplace_back(op, data);
		}

		void add(std::list<base::OpCode> ops) {
			std::stack<base::OpCode> tmp;
			opStack.insert(opStack.end(), ops.begin(), ops.end());
		}

		void set(const std::string& variableName, base::ValueType variableValue) {
			variables[variableName] = variableValue;
		}

		size_t size() const {
			return dataStack.size();
		}

		std::optional<base::ValueType> exec() {
			for (const auto& i : variables) {
				if (!i.second.has_value()) {
					throw std::exception(std::string("Variable not set: " + i.first).c_str());
				}
			}

			while (!opStack.empty()) {
				execNext(opStack.back());
				opStack.pop_back();
			}

			return dataStack.empty() ? std::optional<base::ValueType>(std::nullopt) : std::optional<base::ValueType>(resolve(dataStack.top()));
		}

		std::string toString() {
			return utils::printStack(opStack);
		}

		std::stack<base::StackType> getDataStack() const {
			return dataStack;
		}
	private:
		std::unordered_map<std::string, std::optional<base::ValueType>> variables;
		std::stack<base::StackType> dataStack;
		std::list<base::OpCode> opStack;

		base::ValueType pop() {
			const base::StackType data = dataStack.top();
			dataStack.pop();
			return resolve(data);
		}

		base::ValueType resolve(base::StackType data) const {
			return data.holds<base::ValueType>() ? data.get<base::ValueType>() : variables.at(data.get<std::string>()).value();
		}

		void execNext(base::OpCode op) {
			switch (op.getOperator()) {
				case base::Operator::LOAD: dataStack.push(op.value()); break;
				case base::Operator::STORE: variables[op.value().get<std::string>()] = pop(); break;
				case base::Operator::POP: pop(); break;
				case base::Operator::INCR: executeOP([](base::ValueType a) {return a + 1; }); break;
				case base::Operator::DECR: executeOP([](base::ValueType a) {return a - 1; }); break;
				case base::Operator::EQ: executeOP([](base::ValueType a, base::ValueType b) {return a == b; }); break;
				case base::Operator::UNEQ: executeOP([](base::ValueType a, base::ValueType b) {return a != b; }); break;
				case base::Operator::ADD: executeOP([](base::ValueType a, base::ValueType b) {return a + b; }); break;
				case base::Operator::SUB: executeOP([](base::ValueType a, base::ValueType b) {return a - b; }); break;
				case base::Operator::MULT: executeOP([](base::ValueType a, base::ValueType b) {return a * b; }); break;
				case base::Operator::DIV: executeOP([](base::ValueType a, base::ValueType b) {return a / b; }); break;
				case base::Operator::LABEL: break;
				case base::Operator::JUMP_LABEL_IF:
					if (pop().get<bool>() == false) {
						jumpToLabel(op.value().get<base::ValueType>().get<long>());
					}
					break;
				case base::Operator::JUMP_LABEL: jumpToLabel(op.value().get<base::ValueType>().get<long>()); break;
				default:
					throw ex::Exception("Unrecognized token: "s + base::getName(op.getOperator()));
			}
		}

		void executeOP(std::function<base::ValueType(base::ValueType a, base::ValueType b)> func) {
			base::ValueType a = pop();
			base::ValueType b = pop();
			dataStack.push(func(b, a));
		}

		void executeOP(std::function<base::ValueType(base::ValueType a)> func) {
			base::ValueType a = pop();
			dataStack.push(func(a));
		}

		void jumpToLabel(int index) {
			while (!((opStack.back().getOperator() == base::Operator::LABEL) and (opStack.back().value().get<base::ValueType>().get<long>() == index))) {
				opStack.pop_back();
			}
		}
	};
}