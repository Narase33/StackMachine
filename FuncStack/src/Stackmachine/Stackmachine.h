#pragma once

#include <list>
#include <sstream>
#include <vector>

#include "src/Base/Program.h"
#include "src/Utils/Utils.h"
#include "src/Exception.h"

namespace stackmachine {
	class StackMachine {
	public:
		StackMachine(base::Program toExecute)
			: program(std::move(toExecute)) {
			pc = program.bytecode.begin();
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
					case base::OpCode::POP:
						dataStack.pop_back();
						break;
					case base::OpCode::LOAD_LITERAL:
						dataStack.push_back(program.getConstant(pc->unsignedData()));
						break;
					case base::OpCode::STORE:
						setVariable(pc->unsignedData(), dataStack.back());
						break;
					case base::OpCode::LOAD_VARIABLE:
						dataStack.push_back(getVariable(pc->unsignedData()));
						break;
					case base::OpCode::CREATE_VARIABLE:
						dataStack.push_back(base::BasicType::fromId(static_cast<base::TypeIndex>(pc->unsignedData())));
						break;
					case base::OpCode::JUMP:
						pc += pc->signedData(); // loop will increment +1
						break;
					case base::OpCode::JUMP_IF_NOT:
						if (pop().getBool() == false) {
							pc += pc->signedData(); // loop will increment +1
						}
						break;
					case base::OpCode::BEGIN_SCOPE:
						dataStackScopes.push(dataStack.size());
						break;
					case base::OpCode::END_SCOPE:
						for (int i = 0; i < pc->signedData(); i++) {
							dataStack.erase(dataStack.begin() + dataStackScopes.top(), dataStack.end());
							dataStackScopes.pop();
						}
						break;
						// ==== COMPARE ====
					case base::OpCode::EQ:
						executeOP(std::equal_to());
						break;
					case base::OpCode::UNEQ:
						executeOP(std::not_equal_to());
						break;
					case base::OpCode::LESS:
						executeOP(std::less());
						break;
					case base::OpCode::BIGGER:
						executeOP(std::greater());
						break;
						// ==== MATH ====
					case base::OpCode::INCR:
						executeOP(std::plus(), base::BasicType(1));
						break;
					case base::OpCode::DECR:
						executeOP(std::minus(), base::BasicType(1));
						break;
					case base::OpCode::ADD:
						executeOP(std::plus());
						break;
					case base::OpCode::SUB:
						executeOP(std::minus());
						break;
					case base::OpCode::MULT:
						executeOP(std::multiplies());
						break;
					case base::OpCode::DIV:
						executeOP(std::divides());
						break;
					default:
						throw ex::Exception("Unrecognized token: "s + opCodeName(pc->getOpCode()));
				}
				pc++;
			}
			assert(dataStackScopes.size() == 1);
		}

		std::string toString() const {
			std::ostringstream stream;

			stream << "Literals:\n";
			for (int i = 0; i < program.constants.size(); i++) {
				stream << std::setw(3) << std::right << i << " | " << std::setw(20) << std::left;
				stream << program.constants[i].toString() << " ";
				stream << " (" << idToString(program.constants[i].typeId()) << ")\n";
			}

			stream << "\nStack:\n";
			for (int i = 0; i < dataStack.size(); i++) {
				stream << std::setw(3) << std::right << i << " | " << std::setw(20) << std::left;
				stream << dataStack[i].toString() << " ";
				stream << " (" << idToString(dataStack[i].typeId()) << ")\n";
			}

			stream << "\nByteCode:\n";
			for (int i = 0; i < program.bytecode.size(); i++) {
				base::OpCode opCode = program.bytecode[i].getOpCode();
				const int64_t value = program.bytecode[i].signedData();

				stream << std::setw(3) << std::right << i << " | " << std::setw(20) << std::left << opCodeName(opCode) << " ";
				switch (opCode) {
					case base::OpCode::CREATE_VARIABLE:
						stream << value << " (" << idToString(static_cast<base::TypeIndex>(value)) << ")";
						break;
					case base::OpCode::JUMP: // fallthrough
					case base::OpCode::JUMP_IF_NOT:
						stream << value << " -> " << (i + value);
						break;
					case base::OpCode::LOAD_LITERAL:
						stream << value << " (" << program.constants[value].toString() << ")";
						break;
					case base::OpCode::END_SCOPE: // fallthrough
					case base::OpCode::STORE: // fallthrough
					case base::OpCode::LOAD_VARIABLE:
						stream << value;
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
		base::Program program;
		std::vector<base::Operation>::const_iterator pc;

		base::BasicType pop() {
			base::BasicType data = dataStack.back();
			dataStack.pop_back();
			return data;
		}

		template<typename ExecutionFunction>
		void executeOP(ExecutionFunction func) {
			const base::BasicType a = pop();
			const base::BasicType b = pop();
			dataStack.push_back(func(b, a));
		}

		template<typename ExecutionFunction>
		void executeOP(ExecutionFunction func, const base::BasicType& operand) {
			const base::BasicType a = pop();
			dataStack.push_back(func(a, operand));
		}
	};
}