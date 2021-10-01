#pragma once

#include <type_traits>
#include <variant>

#include "src/Exception.h"

using sm_signed = long;
using sm_unsigned = size_t;
using sm_float = double;
using sm_bool = bool;

template<typename T>
using is_bool = std::is_same<T, bool>;

namespace base {

	class ValueType {
	public:
		explicit ValueType(long long value) : inner((sm_signed)value) {}
		explicit ValueType(long value) : inner((sm_signed)value) {}
		explicit ValueType(int value) : inner((sm_signed)value) {}
		explicit ValueType(size_t value) : inner((sm_unsigned)value) {}
		explicit ValueType(long double value) : inner((sm_float)value) {}
		explicit ValueType(double value) : inner((sm_float)value) {}
		explicit ValueType(float value) : inner((sm_float)value) {}
		explicit ValueType(bool value) : inner((sm_bool)value) {}
		explicit ValueType() {}

		bool isSigned() const {
			return std::holds_alternative<sm_signed>(inner);
		}

		bool isUnsigned() const {
			return std::holds_alternative<sm_unsigned>(inner);
		}

		bool isFloat() const {
			return std::holds_alternative<sm_float>(inner);
		}

		bool isBool() const {
			return std::holds_alternative<sm_bool>(inner);
		}

		sm_signed getSigned() const {
			return std::get<sm_signed>(inner);
		}

		sm_unsigned getUnsigned() const {
			return std::get<sm_unsigned>(inner);
		}

		sm_float getFloat() const {
			return std::get<sm_float>(inner);
		}

		sm_bool getBool() const {
			return std::get<sm_bool>(inner);
		}

		std::string toString() const {
			std::ostringstream o;
			o << *this;
			return o.str();
		}

		// ===== ===== ===== ===== OPERATIONS ===== ===== ===== =====

		friend std::ostream& operator<<(std::ostream& o, const ValueType& type) {
			o << std::boolalpha;
			std::visit([&](auto&& a) {
				o << a;
			}, type.inner);
			return o;
		}

		friend ValueType operator+(const ValueType& a, const ValueType& b) {
			return std::visit([] (const auto& a, const auto& b) {
				ex::assure(!is_bool<decltype(a)>::value, "Unexpected usage of operator+ with bool (first parameter)");
				ex::assure(!is_bool<decltype(b)>::value, "Unexpected usage of operator+ with bool (second parameter)");
				return base::ValueType(a + b);
			}, a.inner, b.inner);
		}

		friend ValueType operator-(const ValueType& a, const ValueType& b) {
			return std::visit([](const auto& a, const auto& b) {
				ex::assure(!is_bool<decltype(a)>::value, "Unexpected usage of operator- with bool (first parameter)");
				ex::assure(!is_bool<decltype(b)>::value, "Unexpected usage of operator- with bool (second parameter)");
				return base::ValueType(a - b);
			}, a.inner, b.inner);
		}

		friend ValueType operator*(const ValueType& a, const ValueType& b) {
			return std::visit([](const auto& a, const auto& b) {
				ex::assure(!is_bool<decltype(a)>::value, "Unexpected usage of operator* with bool (first parameter)");
				ex::assure(!is_bool<decltype(b)>::value, "Unexpected usage of operator* with bool (second parameter)");
				return base::ValueType(a * b);
			}, a.inner, b.inner);
		}

		friend ValueType operator/(const ValueType& a, const ValueType& b) {
			return std::visit([](const auto& a, const auto& b) {
				ex::assure(!is_bool<decltype(a)>::value, "Unexpected usage of operator/ with bool (first parameter)");
				ex::assure(!is_bool<decltype(b)>::value, "Unexpected usage of operator/ with bool (second parameter)");

				if (std::is_same<decltype(b), sm_float>::value) {
					ex::assure(b != 0.0f, "Division through zero");
				} else {
					ex::assure(b != 0, "Division through zero");
				}

				return base::ValueType(a / b);
			}, a.inner, b.inner);
		}

		friend ValueType operator==(const ValueType& a, const ValueType& b) noexcept {
			return std::visit([](const auto& a, const auto& b) {
				return base::ValueType(a == b);
			}, a.inner, b.inner);
		}

		friend ValueType operator!=(const ValueType& a, const ValueType& b) noexcept {
			return std::visit([](const auto& a, const auto& b) {
				return base::ValueType(a != b);
			}, a.inner, b.inner);
		}

	private:
		std::variant<sm_float, sm_signed, sm_unsigned, sm_bool> inner;
	};
}