#pragma once

#include <vector>

#include "Operation.h"

namespace base {
	class LiteralStore {
	public:
		template<typename T>
		size_t push(T value) {
			const auto pos = std::find_if(literals.begin(), literals.end(), [&](const BasicType& b) {
				return (b == BasicType(value)).getBool(); // TODO std::visit?
			});

			const size_t index = std::distance(literals.begin(), pos);
			if (pos == literals.end()) {
				literals.emplace_back(value);
			}

			return index;
		}

		const BasicType& get(size_t index) const {
			assert(literals.size() > index);
			return literals[index];
		}

		size_t size() const {
			return literals.size();
		}

		const BasicType& operator[](size_t index) const {
			return literals[index];
		}

	private:
		std::vector<BasicType> literals;
	};
}