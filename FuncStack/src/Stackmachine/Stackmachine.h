#pragma once

#include <list>
#include <sstream>
#include <vector>

#include "src/Base/Operation.h"
#include "src/Utils/Utils.h"
#include "src/Exception.h"

namespace stackmachine {
	class StackMachine {
	public:
		StackMachine(const std::list<base::Operation>& toExecute) {
			programm.reserve(toExecute.size() + 1);
			std::copy(toExecute.begin(), toExecute.end(), std::back_inserter(programm));
			programm.push_back(base::Operation(base::OpCode::END_PROGRAM));
			pc = programm.begin();
			dataStackScopes.push(0);
		}

		size_t addVariable(base::BasicType variableValue) {
			dataStack.push_back(variableValue);
			return dataStack.size() - 1;
		}

		void setVariable(size_t offset, base::BasicType variableValue) {
			assert(offset < dataStack.size());
			dataStack[offset] = std::move(variableValue);
		}

		base::BasicType getVariable(size_t offset) const {
			assert(offset < dataStack.size());
			return dataStack[offset];
		}

		size_t size() const {
			return dataStack.size();
		}

		void exec() {
			while (pc->getOpCode() != base::OpCode::END_PROGRAM) {
				switch (pc->getOpCode()) {
					// ==== META ====
					case base::OpCode::LITERAL:
						dataStack.push_back(pc->value()); break;
					case base::OpCode::STORE:
					{
						base::BasicType variableValue = pop();
						setVariable(pc->value().getUint(), std::move(variableValue));
					}
					break;
					case base::OpCode::LOAD:
					{
						base::BasicType variableValue = getVariable(pc->value().getUint());
						dataStack.push_back(std::move(variableValue));
					}
					break;
					case base::OpCode::CREATE:
					{
						const base::sm_uint typeId = pc->value().getUint();
						dataStack.push_back(base::BasicType::fromId(static_cast<base::TypeIndex>(typeId)));
					}
					break;
					case base::OpCode::POP:
						pop(); break;
					case base::OpCode::JUMP:
						pc += pc->value().getInt() - 1; // loop will increment +1
						break;
					case base::OpCode::JUMP_IF_NOT:
						if (pop().getBool() == false) {
							pc += pc->value().getInt() - 1; // loop will increment +1
						}
						break;
					case base::OpCode::BEGIN_SCOPE:
						dataStackScopes.push(dataStack.size());
						break;
					case base::OpCode::END_SCOPE:
						dataStack.erase(dataStack.begin() + dataStackScopes.top(), dataStack.end());
						dataStackScopes.pop();
						break;
						// ==== COMPARE ====
					case base::OpCode::EQ:
						executeOP(std::equal_to()); break;
					case base::OpCode::UNEQ:
						executeOP(std::not_equal_to()); break;
					case base::OpCode::LESS:
						executeOP(std::less()); break;
					case base::OpCode::BIGGER:
						executeOP(std::greater()); break;
						// ==== MATH ====
					case base::OpCode::INCR:
						executeOP(std::plus(), base::BasicType(1)); break;
					case base::OpCode::DECR:
						executeOP(std::minus(), base::BasicType(1)); break;
					case base::OpCode::ADD:
						executeOP(std::plus()); break;
					case base::OpCode::SUB:
						executeOP(std::minus()); break;
					case base::OpCode::MULT:
						executeOP(std::multiplies()); break;
					case base::OpCode::DIV:
						executeOP(std::divides()); break;
					default:
						throw ex::Exception("Unrecognized token: "s + opCodeName(pc->getOpCode()));
				}
				pc++;
			}
		}

		std::string toString() const {
			std::ostringstream stream;

			stream << "Programm:" << std::endl;
			for (int i = 0; i < programm.size(); i++) {
				base::OpCode opCode = programm[i].getOpCode();
				base::BasicType value = programm[i].value();

				stream << std::setw(3) << std::right << i << " | " << std::setw(20) << std::left << opCodeName(opCode) << " ";
				switch (opCode) {
					case base::OpCode::CREATE:
						stream << value.toString() << " (" << idToString(static_cast<base::TypeIndex>(value.getUint())) << ")";
						break;
					case base::OpCode::JUMP: // fallthrough
					case base::OpCode::JUMP_IF_NOT:
						stream << value.toString() << " (" << (i + value.getInt()) << ")";
						break;
					case base::OpCode::LITERAL: // fallthrough
					case base::OpCode::STORE: // fallthrough
					case base::OpCode::LOAD:
						stream << value.toString();
						break;
				}

				stream << "\n";
			}

			return stream.str();
		}

		const std::vector<base::BasicType>& getDataStack() const {
			return dataStack;
		}

	private:
		std::stack<size_t, std::vector<size_t>> dataStackScopes;
		std::vector<base::BasicType> dataStack;
		std::vector<base::Operation> programm;
		std::vector<base::Operation>::const_iterator pc;

		base::BasicType pop() {
			base::BasicType data = dataStack.back();
			dataStack.pop_back();
			return data;
		}

		void executeOP(std::function<base::BasicType(const base::BasicType& a, const base::BasicType& b)> func) {
			const base::BasicType a = pop();
			const base::BasicType b = pop();
			dataStack.push_back(func(b, a));
		}

		void executeOP(std::function<base::BasicType(const base::BasicType& a, const base::BasicType& b)> func, const base::BasicType& operand) {
			const base::BasicType a = pop();
			dataStack.push_back(func(a, operand));
		}
	};
}