#pragma once

#include <stack>
#include <sstream>
#include <algorithm>

using namespace std::string_literals;

namespace utils {
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

	template<typename T, typename T2>
	constexpr bool noneOf(T t, T2 t2) {
		static_assert(std::is_same<T, T2>::value);
		return (t != t2);
	}

	template<typename T, typename T2, typename... T3>
	constexpr bool noneOf(T t, T2 t2, T3... t3) {
		static_assert(std::is_same<T, T2>::value);
		return (t != t2) and noneOf(t, t3...);
	}
}