#pragma once

#include <string>
#include <sstream>


namespace base {
	namespace {
		template <typename T>
		void _toString(std::stringstream& stream, const T& t) {
			stream << t;
		}

		template <typename T1, typename... T2>
		void _toString(std::stringstream& stream, const T1& t1, const T2... t2) {
			stream << t1;
			_toString(stream, t2);
		}
	}

	template <typename T>
	std::string toString(const T& t) {
		std::stringstream stream;
		_toString(stream, t);
		return stream.str();
	}

	template <typename... T>
	std::string toString(const T&... t) {
		std::stringstream stream;
		_toString(stream, t);
		return stream.str();
	}
}

