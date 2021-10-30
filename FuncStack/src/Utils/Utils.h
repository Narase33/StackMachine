#pragma once

#include <stack>
#include <sstream>
#include <algorithm>

using namespace std::string_literals;

namespace utils {
	template<class... Ts>
	struct Overloaded : Ts... {
		using Ts::operator()...;
	};

	template<class... Ts>
	Overloaded(Ts...)->Overloaded<Ts...>;

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

	template<typename T>
	bool none_of(T t, std::initializer_list<T> list) {
		return std::none_of(list.begin(), list.end(), [&](const T& e) {return e == t; });
	}

	template<typename T>
	bool any_of(T t, std::initializer_list<T> list) {
		return std::any_of(list.begin(), list.end(), [&](const T& e) {return e == t; });
	}

	template<typename T, typename T2>
	constexpr bool anyOf(T t, T2 t2) {
		static_assert(std::is_same<T, T2>::value);
		return (t == t2);
	}

	template<typename T, typename T2, typename... T3>
	constexpr bool anyOf(T t, T2 t2, T3... t3) {
		static_assert(std::is_same<T, T2>::value);
		return (t == t2) or anyOf(t, t3...);
	}
}