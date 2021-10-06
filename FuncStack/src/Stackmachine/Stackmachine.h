#pragma once

#include <list>
#include <sstream>
#include <vector>

#include "src/Base/Operation.h"
#include "src/Utils/Utils.h"
#include "src/Exception.h"
#include "Scope.h"

namespace stackmachine {
	class StackMachine {
	public:
		StackMachine(std::list<base::Operation> toExecute) {
			programm.reserve(toExecute.size() + 1);
			std::move(toExecute.begin(), toExecute.end(), std::back_inserter(programm));
			programm.push_back(base::Operation(base::Operator::END));
			pc = programm.begin();
		}

		void add(const std::string& variableName, base::BasicType variableValue) {
			variables.add(variableName, variableValue);
		}

		void set(const std::string& variableName, base::BasicType variableValue) {
			variables.set(variableName, variableValue);
		}

		size_t size() const {
			return dataStack.size();
		}

		std::optional<base::StackFrame> exec() {
			/*for (const auto& i : variables) {
				if (!i.second.has_value()) {
					throw std::exception(std::string("Variable not set: " + i.first).c_str());
				}
			}*/

			while ((pc->getOperator() != base::Operator::END) && (pc != programm.end())) {
				execNext();
				pc++;
			}

			return dataStack.empty() ? std::optional<base::StackFrame>(std::nullopt) : std::optional<base::StackFrame>(dataStack.top());
		}

		std::string toString() const {
			std::ostringstream stream;

			stream << "Programm:" << std::endl;
			for (const base::Operation& i : programm) {
				std::string opValue;
				if (i.hasStackFrame(0)) {
					const base::StackFrame& value = i.getStackFrame(0);
					opValue = " " + (value.isVariable() ? value.getName() : value.getValue().toString());
				}

				stream << "\t" << getName(i.getOperator()) << opValue << std::endl;
			}

			return stream.str();
		}

		std::stack<base::StackFrame> getDataStack() const {
			return dataStack;
		}

	private:
		Scope variables;
		std::stack<base::StackFrame> dataStack;
		std::vector<base::Operation> programm;
		std::vector<base::Operation>::const_iterator pc;

		base::StackFrame pop() {
			base::StackFrame data = dataStack.top();
			dataStack.pop();
			return data;
		}

		base::BasicType resolve(const base::StackFrame& data) const {
			return data.isValue() ? data.getValue() : *(variables.get(data.getName()));
		}

		void execNext() {
			switch (pc->getOperator()) {
				case base::Operator::END:
					break;
				case base::Operator::LOAD:
					dataStack.push(pc->getStackFrame(0));
					break;
				case base::Operator::STORE:
				{
					const base::StackFrame variableValue = pop();
					const base::StackFrame variableName = pop();
					variables.set(variableName.getName(), resolve(variableValue));
				}
					break;
				case base::Operator::CREATE:
				{
					const base::StackFrame variableType = pop();
					const base::StackFrame variableName = pop();
					const sm_uint typeId = variableType.getValue().getUint();
					variables.add(variableName.getName(), base::BasicType::idToType(typeId));
				}
					break;
				case base::Operator::POP:
					pop();
					break;
				case base::Operator::INCR:
					executeOP(std::plus(), base::BasicType(1));
					break;
				case base::Operator::DECR:
					executeOP(std::minus(), base::BasicType(1));
					break;
				case base::Operator::EQ:
					executeOP(std::equal_to());
					break;
				case base::Operator::UNEQ:
					executeOP(std::not_equal_to());
					break;
				case base::Operator::ADD:
					executeOP(std::plus());
					break;
				case base::Operator::SUB:
					executeOP(std::minus());
					break;
				case base::Operator::MULT:
					executeOP(std::multiplies());
					break;
				case base::Operator::DIV:
					executeOP(std::divides());
					break;
				case base::Operator::JUMP:
					pc += pc->getStackFrame(0).getValue().getInt() - 1; // loop will increment +1
					break;
				case base::Operator::JUMP_IF:
					if (pop().getValue().getBool() == false) {
						pc += pc->getStackFrame(0).getValue().getInt() - 1; // loop will increment +1
					}
					break;
				case base::Operator::BRACKET_OPEN:
					variables.newScope();
					break;
				case base::Operator::BRACKET_CLOSE:
					variables.leaveScope();
					break;
				default:
					throw ex::Exception("Unrecognized token: "s + base::getName(pc->getOperator()));
			}
		}

		void executeOP(std::function<base::BasicType(const base::BasicType& a, const base::BasicType& b)> func) {
			const base::StackFrame a = pop();
			const base::StackFrame b = pop();
			dataStack.emplace(func(resolve(b), resolve(a)));
		}

		void executeOP(std::function<base::BasicType(const base::BasicType& a, const base::BasicType& b)> func, const base::BasicType& operand) {
			const base::StackFrame a = pop();
			dataStack.emplace(func(resolve(a), operand));
		}
	};
}