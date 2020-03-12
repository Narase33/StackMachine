#pragma once

#include <stack>
#include <sstream>
#include <algorithm>

#include "src/Base/OpCode.h"

namespace utils {
	template<typename T>
	void transfer_single(std::stack<T>& from, std::stack<T>& to) {
		to.push(from.top());
		from.pop();
	}

	template<typename T>
	void transfer(std::stack<T>& from, std::stack<T>& to) {
		while (!from.empty()) {
			transfer_single<T>(from, to);
		}
	}

	template<typename T>
	void transfer(std::stack<T>&& from, std::stack<T>& to) {
		while (!from.empty()) {
			transfer_single<T>(from, to);
		}
	}

	std::string printStack(const std::list<base::StackType>& s) {
		using namespace base;
		std::stack<StackType> copy;
		std::ostringstream stream;

		stream << "Stack:" << std::endl;
		for (const auto& i : s) {
			if (i.holds<ValueType>()) {
				stream << "\t" << i.get<ValueType>() << std::endl;
			} else {
				stream << "\t" << i.get<std::string>() << std::endl;
			}
		}
		
		return stream.str();
	}

	std::string printStack(const std::list<base::OpCode>& s) {
		using namespace base;
		std::stack<OpCode> copy;
		std::ostringstream stream;


		stream << "Stack:" << std::endl;
		for (const auto& i : s) {
			std::string opValue;
			if (i.hasValue()) {
				const auto& value = i.value();
				opValue = " " + (value.holds<std::string>() ? value.get<std::string>() : value.get<ValueType>().toString());
			}

			stream << "\t" << getName(i.getOperator()) << opValue << std::endl;
		}

		return stream.str();
	}

	template<typename T>
	bool none_of(T t, std::initializer_list<T> list) {
		return std::none_of(list.begin(), list.end(), [&](const T& e) {return e == t; });
	}

	template<typename T>
	bool any_of(T t, std::initializer_list<T> list) { // TODO Variadic
		return std::any_of(list.begin(), list.end(), [&](const T& e) {return e == t; });
	}
}