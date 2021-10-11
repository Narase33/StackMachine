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
		StackMachine(const std::list<base::Operation>& toExecute) {
			programm.reserve(toExecute.size() + 1);
			std::copy(toExecute.begin(), toExecute.end(), std::back_inserter(programm));
			programm.push_back(base::Operation(OpCode::END_PROGRAM));
			pc = programm.begin();
		}

		void add(const std::string& variableName, base::BasicType variableValue) {
			variables.add(variableName, variableValue);
		}

		void set(const std::string& variableName, base::BasicType variableValue) {
			variables.set(variableName, variableValue);
		}

		base::BasicType get(const std::string& variableName) const {
			return *variables.get(variableName);
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

			while (pc->getOpCode() != OpCode::END_PROGRAM) {
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

				const base::StackFrame& value1 = i.firstValue();
				opValue = " " + (value1.isVariable() ? value1.getName() : value1.getValue().toString());

				const base::StackFrame& value2 = i.secondValue();
				opValue += " " + (value2.isVariable() ? value2.getName() : value2.getValue().toString());

				stream << "\t" << opCodeName(i.getOpCode()) << opValue << std::endl;
			}

			return stream.str();
		}

		std::stack<base::BasicType> getDataStack() const {
			return dataStack;
		}

	private:
		Scope variables;
		std::stack<base::BasicType> dataStack;
		std::vector<base::Operation> programm;
		std::vector<base::Operation>::const_iterator pc;

		base::BasicType pop() {
			base::BasicType data = dataStack.top();
			dataStack.pop();
			return data;
		}

		base::BasicType resolve(const base::StackFrame& data) const {
			return data.isValue() ? data.getValue() : *(variables.get(data.getName()));
		}

		void execNext() {
			switch (pc->getOpCode()) {
				case OpCode::LOAD:
					dataStack.push(resolve(pc->firstValue()));
					break;
				case OpCode::STORE:
				{
					base::BasicType variableValue = pop();
					variables.set(pc->firstValue().getName(), std::move(variableValue));
				}
				break;
				case OpCode::CREATE:
				{
					const sm_uint typeId = pc->firstValue().getValue().getUint();
					const std::string& variableName = pc->secondValue().getName();
					variables.add(variableName, base::BasicType::idToType(typeId));
				}
				break;
				case OpCode::POP:
					pop();
					break;
				case OpCode::INCR:
					executeOP(std::plus(), base::BasicType(1));
					break;
				case OpCode::DECR:
					executeOP(std::minus(), base::BasicType(1));
					break;
				case OpCode::EQ:
					executeOP(std::equal_to());
					break;
				case OpCode::UNEQ:
					executeOP(std::not_equal_to());
					break;
				case OpCode::ADD:
					executeOP(std::plus());
					break;
				case OpCode::SUB:
					executeOP(std::minus());
					break;
				case OpCode::MULT:
					executeOP(std::multiplies());
					break;
				case OpCode::DIV:
					executeOP(std::divides());
					break;
				case OpCode::JUMP:
					pc += pc->firstValue().getValue().getInt() - 1; // loop will increment +1
					break;
				case OpCode::JUMP_IF:
					if (pop().getBool() == false) {
						pc += pc->firstValue().getValue().getInt() - 1; // loop will increment +1
					}
					break;
				case OpCode::BRACKET_CURLY_OPEN:
					variables.newScope();
					break;
				case OpCode::BRACKET_CURLY_CLOSE:
					variables.leaveScope();
					break;
				default:
					throw ex::Exception("Unrecognized token: "s + opCodeName(pc->getOpCode()));
			}
		}

		void executeOP(std::function<base::BasicType(const base::BasicType& a, const base::BasicType& b)> func) {
			const base::BasicType a = pop();
			const base::BasicType b = pop();
			dataStack.emplace(func(b, a));
		}

		void executeOP(std::function<base::BasicType(const base::BasicType& a, const base::BasicType& b)> func, const base::BasicType& operand) {
			const base::BasicType a = pop();
			dataStack.emplace(func(a, operand));
		}
	};
}