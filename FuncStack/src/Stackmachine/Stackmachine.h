#pragma once

#include <list>
#include <sstream>
#include <vector>

#include "src/Base/StackFrame.h"
#include "src/Utils/Utils.h"
#include "src/Exception.h"
#include "Scope.h"

namespace stackmachine {
	class StackMachine {
	public:
		StackMachine() = default;

		StackMachine(std::list<base::StackFrame> toExecute) {
			programm.reserve(toExecute.size() + 1);
			std::move(toExecute.begin(), toExecute.end(), std::back_inserter(programm));
			programm.push_back(base::StackFrame(base::Operator::END));
			pc = programm.begin();
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

			while ((pc->getOperator() != base::Operator::END) && (pc != programm.end())) {
				execNext();
				pc++;
			}

			return dataStack.empty() ? std::optional<base::ValueType>(std::nullopt) : std::optional<base::ValueType>(resolve(dataStack.top()));
		}

		std::string toString() const {
			std::ostringstream stream;

			stream << "Programm:" << std::endl;
			for (const base::StackFrame& i : programm) {
				std::string opValue;
				if (i.hasValue()) {
					const base::StackType& value = i.value();
					opValue = " " + (value.isVariable() ? value.getVariableName() : value.getValue().toString());
				}

				stream << "\t" << getName(i.getOperator()) << opValue << std::endl;
			}

			return stream.str();
		}

		std::stack<base::StackType> getDataStack() const {
			return dataStack;
		}

	private:
		std::unordered_map<std::string, std::optional<base::ValueType>> variables;
		std::stack<base::StackType> dataStack;
		std::vector<base::StackFrame> programm;
		std::vector<base::StackFrame>::const_iterator pc;

		base::ValueType pop() {
			const base::StackType data = dataStack.top();
			dataStack.pop();
			return resolve(data);
		}

		base::ValueType resolve(base::StackType data) const {
			return data.isValue() ? data.getValue() : variables.at(data.getVariableName()).value();
		}

		void execNext() {
			switch (pc->getOperator()) {
				case base::Operator::END: break;
				case base::Operator::LOAD: dataStack.push(pc->value()); break;
				case base::Operator::STORE: variables[pc->value().getVariableName()] = pop(); break;
				case base::Operator::POP: pop(); break;
				case base::Operator::INCR: executeOP(std::plus(), base::ValueType(1)); break;
				case base::Operator::DECR: executeOP(std::minus(), base::ValueType(1)); break;
				case base::Operator::EQ: executeOP(std::equal_to()); break;
				case base::Operator::UNEQ: executeOP(std::not_equal_to()); break;
				case base::Operator::ADD: executeOP(std::plus()); break;
				case base::Operator::SUB: executeOP(std::minus()); break;
				case base::Operator::MULT: executeOP(std::multiplies()); break;
				case base::Operator::DIV: executeOP(std::divides()); break;
				case base::Operator::JUMP:
					pc += pc->value().getValue().getSigned() - 1; // loop will increment +1
					break;
				case base::Operator::JUMP_IF:
					if (pop().getBool() == false) {
						pc += pc->value().getValue().getSigned() - 1; // loop will increment +1
					}
					break;
				default:
					throw ex::Exception("Unrecognized token: "s + base::getName(pc->getOperator()));
			}
		}

		void executeOP(std::function<base::ValueType(base::ValueType a, base::ValueType b)> func) {
			base::ValueType a = pop();
			base::ValueType b = pop();
			dataStack.push(base::StackType(func(b, a)));
		}

		void executeOP(std::function<base::ValueType(base::ValueType a, base::ValueType b)> func, const base::ValueType& operand) {
			base::ValueType a = pop();
			dataStack.push(base::StackType(func(a, operand)));
		}
	};
}