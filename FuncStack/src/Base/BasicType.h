#pragma once

#include <type_traits>
#include <variant>
#include <string_view>

#include "src/Exception.h"

using sm_int = long;
using sm_uint = size_t;
using sm_float = double;
using sm_bool = bool;

template<typename T> using is_int = std::is_same<T, sm_int>;
template<typename T> using is_uint = std::is_same<T, sm_uint>;
template<typename T> using is_float = std::is_same<T, sm_float>;
template<typename T> using is_bool = std::is_same<T, sm_bool>;

namespace base {

	class BasicType {
	public:
		explicit BasicType(long long value) : inner((sm_int)value) {}
		explicit BasicType(long value) : inner((sm_int)value) {}
		explicit BasicType(int value) : inner((sm_int)value) {}
		explicit BasicType(size_t value) : inner((sm_uint)value) {}
		explicit BasicType(long double value) : inner((sm_float)value) {}
		explicit BasicType(double value) : inner((sm_float)value) {}
		explicit BasicType(float value) : inner((sm_float)value) {}
		explicit BasicType(bool value) : inner((sm_bool)value) {}
		explicit BasicType() {}

		constexpr bool isInt() const {
			return std::holds_alternative<sm_int>(inner);
		}

		constexpr bool isUint() const {
			return std::holds_alternative<sm_uint>(inner);
		}

		constexpr bool isFloat() const {
			return std::holds_alternative<sm_float>(inner);
		}

		constexpr bool isBool() const {
			return std::holds_alternative<sm_bool>(inner);
		}

		constexpr sm_int getInt() const {
			return std::get<sm_int>(inner);
		}

		constexpr sm_int& getInt() {
			return std::get<sm_int>(inner);
		}

		constexpr sm_uint getUint() const {
			return std::get<sm_uint>(inner);
		}

		constexpr sm_uint& getUint() {
			return std::get<sm_uint>(inner);
		}

		constexpr sm_float getFloat() const {
			return std::get<sm_float>(inner);
		}

		constexpr sm_float& getFloat() {
			return std::get<sm_float>(inner);
		}

		constexpr sm_bool getBool() const {
			return std::get<sm_bool>(inner);
		}

		constexpr sm_bool& getBool() {
			return std::get<sm_bool>(inner);
		}

		constexpr size_t typeId() const {
			return inner.index();
		}

		std::string toString() const {
			std::ostringstream o;
			o << *this;
			return o.str();
		}

		// ===== ===== ===== ===== OPERATIONS ===== ===== ===== =====

		friend std::ostream& operator<<(std::ostream& o, const BasicType& type) {
			o << std::boolalpha;
			std::visit([&](auto&& a) {
				o << "[type: " << BasicType::idToString(type.typeId()) << ", value: " << a << "]";
			}, type.inner);
			return o;
		}

		friend BasicType operator+(const BasicType& a, const BasicType& b) {
			return std::visit([](const auto& a, const auto& b) {
				ex::assure(!is_bool<decltype(a)>::value, "Unexpected usage of operator+ with bool (first parameter)");
				ex::assure(!is_bool<decltype(b)>::value, "Unexpected usage of operator+ with bool (second parameter)");
				return base::BasicType(a + b);
			}, a.inner, b.inner);
		}

		friend BasicType operator-(const BasicType& a, const BasicType& b) {
			return std::visit([](const auto& a, const auto& b) {
				ex::assure(!is_bool<decltype(a)>::value, "Unexpected usage of operator- with bool (first parameter)");
				ex::assure(!is_bool<decltype(b)>::value, "Unexpected usage of operator- with bool (second parameter)");
				return base::BasicType(a - b);
			}, a.inner, b.inner);
		}

		friend BasicType operator*(const BasicType& a, const BasicType& b) {
			return std::visit([](const auto& a, const auto& b) {
				ex::assure(!is_bool<decltype(a)>::value, "Unexpected usage of operator* with bool (first parameter)");
				ex::assure(!is_bool<decltype(b)>::value, "Unexpected usage of operator* with bool (second parameter)");
				return base::BasicType(a * b);
			}, a.inner, b.inner);
		}

		friend BasicType operator/(const BasicType& a, const BasicType& b) {
			return std::visit([](const auto& a, const auto& b) {
				ex::assure(!is_bool<decltype(a)>::value, "Unexpected usage of operator/ with bool (first parameter)");
				ex::assure(!is_bool<decltype(b)>::value, "Unexpected usage of operator/ with bool (second parameter)");

				if (std::is_same<decltype(b), sm_float>::value) {
					ex::assure(b != 0.0f, "Division through zero");
				} else {
					ex::assure(b != 0, "Division through zero");
				}

				return base::BasicType(a / b);
			}, a.inner, b.inner);
		}

		friend BasicType operator==(const BasicType& a, const BasicType& b) noexcept {
			return std::visit([](const auto& a, const auto& b) {
				return base::BasicType(a == b);
			}, a.inner, b.inner);
		}

		friend BasicType operator!=(const BasicType& a, const BasicType& b) noexcept {
			return std::visit([](const auto& a, const auto& b) {
				return base::BasicType(a != b);
			}, a.inner, b.inner);
		}

		// ===== ===== ===== ===== META ===== ===== ===== =====

		static BasicType idToType(size_t id) {
			switch (id) {
				case 0: return BasicType(sm_int{});
				case 1: return BasicType(sm_uint{});
				case 2: return BasicType(sm_float{});
				case 3: return BasicType(sm_bool{});
				default: throw std::exception("Unknown type id");
			}
		}

		static std::string idToString(size_t id) {
			switch (id) {
				case 0: return "int";
				case 1: return "uint";
				case 3: return "float";
				case 4: return "bool";
			}
			return "";
		}

		static size_t stringToId(const std::string_view name) {
			if (name == "int") return 0;
			if (name == "uint") return 1;
			if (name == "float") return 2;
			if (name == "bool") return 3;
			return -1;
		}

	private:
		std::variant<sm_float, sm_int, sm_uint, sm_bool> inner;
	};
}