#pragma once

#include <type_traits>
#include <variant>
#include <string_view>
#include <ctype.h>

#include "src/Exception.h"
#include "AtomicTypes.h"	

namespace base {
	class BasicType final {
	public:
		explicit BasicType(long long value) : inner((sm_int)value) {}
		explicit BasicType(long value) : inner((sm_int)value) {}
		explicit BasicType(int value) : inner((sm_int)value) {}
		explicit BasicType(unsigned long long value) : inner((sm_uint)value) {}
		explicit BasicType(unsigned long value) : inner((sm_uint)value) {}
		explicit BasicType(unsigned int value) : inner((sm_uint)value) {}
		explicit BasicType(long double value) : inner((sm_float)value) {}
		explicit BasicType(double value) : inner((sm_float)value) {}
		explicit BasicType(float value) : inner((sm_float)value) {}
		explicit BasicType(bool value) : inner((sm_bool)value) {}
		explicit BasicType() {}

		static BasicType fromId(TypeIndex opCode) {
			switch (opCode) {
				case TypeIndex::Int: return BasicType(sm_int{});
				case TypeIndex::Uint: return BasicType(sm_uint{});
				case TypeIndex::Float: return BasicType(sm_float{});
				case TypeIndex::Bool: return BasicType(sm_bool{});
				default: throw std::exception("Unknown type id");
			}
		}

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

		constexpr TypeIndex typeId() const {
			return static_cast<TypeIndex>(inner.index());
		}

		std::string toString() const {
			std::ostringstream o;
			std::visit([&](const auto& a) { o << a; }, inner);
			return o.str();
		}

		// ===== ===== ===== ===== OPERATIONS ===== ===== ===== =====

		friend std::ostream& operator<<(std::ostream& o, const BasicType& type) {
			o << std::boolalpha;
			std::visit([&](auto&& a) {
				o << "[type: " << idToString(type.typeId()) << ", value: " << a << "]";
			}, type.inner);
			return o;
		}

		friend BasicType operator+(const BasicType& a, const BasicType& b) {
			return std::visit([](const auto& a, const auto& b) {
				ex::assume(!std::is_same<bool, decltype(a)>::value, "Unexpected usage of operator+ with bool (first parameter)");
				ex::assume(!std::is_same<bool, decltype(b)>::value, "Unexpected usage of operator+ with bool (second parameter)");
				return BasicType(a + b);
			}, a.inner, b.inner);
		}

		friend BasicType operator-(const BasicType& a, const BasicType& b) {
			return std::visit([](const auto& a, const auto& b) {
				ex::assume(!std::is_same<bool, decltype(a)>::value, "Unexpected usage of operator- with bool (first parameter)");
				ex::assume(!std::is_same<bool, decltype(b)>::value, "Unexpected usage of operator- with bool (second parameter)");
				return BasicType(a - b);
			}, a.inner, b.inner);
		}

		friend BasicType operator*(const BasicType& a, const BasicType& b) {
			return std::visit([](const auto& a, const auto& b) {
				ex::assume(!std::is_same<bool, decltype(a)>::value, "Unexpected usage of operator* with bool (first parameter)");
				ex::assume(!std::is_same<bool, decltype(b)>::value, "Unexpected usage of operator* with bool (second parameter)");
				return BasicType(a * b);
			}, a.inner, b.inner);
		}

		friend BasicType operator/(const BasicType& a, const BasicType& b) {
			return std::visit([](const auto& a, const auto& b) {
				ex::assume(!std::is_same<bool, decltype(a)>::value, "Unexpected usage of operator/ with bool (first parameter)");
				ex::assume(!std::is_same<bool, decltype(b)>::value, "Unexpected usage of operator/ with bool (second parameter)");

				if (std::is_same<decltype(b), sm_float>::value) {
					ex::assume(b != 0.0f, "Division through zero");
				} else {
					ex::assume(b != 0, "Division through zero");
				}

				return BasicType(a / b);
			}, a.inner, b.inner);
		}

		friend BasicType operator!(const BasicType& a) {
			return BasicType(!a.getBool());
		}

		friend BasicType operator<(const BasicType& a, const BasicType& b) noexcept {
			return std::visit([](const auto& a, const auto& b) {
				return BasicType(a < b);
			}, a.inner, b.inner);
		}

		friend BasicType operator>(const BasicType& a, const BasicType& b) noexcept {
			return std::visit([](const auto& a, const auto& b) {
				return BasicType(a > b);
			}, a.inner, b.inner);
		}

		friend BasicType operator==(const BasicType& a, const BasicType& b) noexcept {
			if (a.typeId() != b.typeId()) {
				return BasicType(false);
			}

			return std::visit([](const auto& a, const auto& b) {
				return BasicType(a == b);
			}, a.inner, b.inner);
		}

		friend BasicType operator!=(const BasicType& a, const BasicType& b) noexcept {
			return !(a == b);
		}

	private:
		std::variant<sm_int, sm_uint, sm_float, sm_bool> inner;
	};
}