#pragma once

#include <sstream>
#include <type_traits>

#include "src/Utils/Variant.h"

namespace base {
	class ValueType;

	using StackType = utils::Variant<ValueType, std::string>;

	class ValueType {
	public:
		ValueType(long double value) : inner((double) value) {}
		ValueType(double value) : inner((double) value) {}
		ValueType(float value) : inner((double) value) {}
		ValueType(long long value) : inner((long) value) {}
		ValueType(long value) : inner((long) value) {}
		ValueType(int value) : inner((long) value) {}
		ValueType(bool value) : inner((bool) value) {}
		ValueType() {}

		std::string toString() const {
			std::ostringstream o;
			o << *this;
			return o.str();
		}

		template <typename T> T get() const { return inner.get<T>(); }
		template <typename T> bool holds() const { return inner.holds<T>(); }

	private:
		utils::Variant<double, long, bool> inner;

		friend std::ostream& operator<<(std::ostream& o, ValueType type);
		friend ValueType operator+(ValueType a, ValueType b);
		friend ValueType operator-(ValueType a, ValueType b);
		friend ValueType operator*(ValueType a, ValueType b);
		friend ValueType operator/(ValueType a, ValueType b);
		friend bool operator==(ValueType other, ValueType b);
		friend bool operator!=(ValueType other, ValueType b);
	};

	namespace {
		struct Add {
			template <typename A, typename B> ValueType operator()(A a, B b) const {
				return ValueType(a + b);
			}

			template <typename A, typename B> ValueType operator()(bool a, bool b) const {
				throw std::exception("Unexpected usage of operator+ with bool");
			}
		};
		struct Sub {
			template <typename A, typename B> ValueType operator()(A a, B b) const {
				return ValueType(a - b);
			}

			template <typename A, typename B> ValueType operator()(bool a, bool b) const {
				throw std::exception("Unexpected usage of operator- with bool");
			}
		};
		struct Mult {
			template <typename A, typename B> ValueType operator()(A a, B b) const {
				return ValueType(a * b);
			}

			template <typename A, typename B> ValueType operator()(bool a, bool b) const {
				throw std::exception("Unexpected usage of operator* with bool");
			}
		};
		struct Div {
			template <typename A, typename B> ValueType operator()(A a, B b) const {
				if (b == 0) {
					throw std::exception("Division through zero");
				}

				return ValueType(a / b);
			}

			template <typename A, typename B> ValueType operator()(bool a, bool b) const {
				throw std::exception("Unexpected usage of operator/ with bool");
			}
		};
		struct Equal {
			template <typename A, typename B> bool operator()(A a, B b) const {
				return a == b;
			}
		};
	} // namespace

	std::ostream& operator<<(std::ostream& o, ValueType type) {
		o << std::boolalpha;
		std::visit([&](auto&& a) { o << a; }, type.inner.inner());
		return o;
	}

	ValueType operator+(ValueType a, ValueType b) {
		return std::visit(Add(), a.inner.inner(), b.inner.inner());
	}
	ValueType operator-(ValueType a, ValueType b) {
		return std::visit(Sub(), a.inner.inner(), b.inner.inner());
	}
	ValueType operator*(ValueType a, ValueType b) {
		return std::visit(Mult(), a.inner.inner(), b.inner.inner());
	}
	ValueType operator/(ValueType a, ValueType b) {
		return std::visit(Div(), a.inner.inner(), b.inner.inner());
	}
	bool operator==(ValueType a, ValueType b) {
		return std::visit(Equal(), a.inner.inner(), b.inner.inner());
	}
	bool operator!=(ValueType a, ValueType b) {
		return !std::visit(Equal(), a.inner.inner(), b.inner.inner());
	}
} // namespace base