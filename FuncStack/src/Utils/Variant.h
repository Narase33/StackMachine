#pragma once

#include <variant>
#include <type_traits>
#include <functional>

namespace utils {
	template <class... T >
	class Variant {
	public:
		constexpr Variant() { /* empty */ }

		template <class Type>
		constexpr Variant(Type v) : value(v) { /* empty */ }

		constexpr Variant(const Variant<T...>& other) : value(other.value) { /*empty */ }

		constexpr Variant(Variant&& other) = default;

		constexpr Variant& operator=(const Variant& other) {
			this->value = other.value;
			return *this;
		}

		template<typename Type>
		constexpr bool holds() const {
			return std::holds_alternative<Type>(value);
		}

		constexpr size_t index() const {
			return value.index();
		}

		constexpr bool isValid() {
			return !value.valueless_by_exception;
		}

		template<typename Type>
		constexpr bool verify(std::function<bool(const Type& t)> operator_test) const {
			return holds<Type>() and operator_test(std::get<Type>(value));
		}

		template<typename Type>
		constexpr Type& get() {
			return std::get<Type>(value);
		}

		template<typename Type>
		constexpr Type get() const {
			return std::get<Type>(value);
		}

		constexpr std::variant<T...> inner() const {
			return value;
		}

		constexpr bool operator==(const Variant& other) const {
			return this->value == other.value;
		}

		constexpr bool operator!=(const Variant& other) const {
			return this->value != other.value;
		}

		std::string toString() const {
			std::ostringstream o;
			o << *this;
			return o.str();
		}

		friend std::ostream& operator<<(std::ostream& o, const Variant& type) {
			o << std::boolalpha;
			std::visit([&](auto&& a) {
				o << a;
			}, type.inner());
			return o;
		}
	private:
		std::variant<T...> value;
	};
}