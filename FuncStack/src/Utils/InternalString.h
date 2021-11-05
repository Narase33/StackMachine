#pragma once

#include <vector>

#include "cString.h"

class InternalString {
	InternalString(cString str) {
		strings.push_back(std::string(str.str, str.length);
		id = strings.size() - 1;
	}

	InternalString(std::string str) {
		const size_t existingId = find(str);
		if (existingId != -1) {
			id = existingId;
		} else {
			strings.push_back(std::move(str));
			id = strings.size() - 1;
		}
	}

	const std::string& str() const {
		return strings[id];
	}

	std::string& str() {
		return strings[id];
	}

	bool operator==(InternalString other) const {
		return this->id == other.id;
	}

private:
	size_t id;
	static std::vector<std::string> strings;

	size_t find(const std::string& str) const {
		const auto pos = std::find(strings.begin(), strings.end(), str);
		if (pos != str) {
			return std::distance(strings.begin(), pos);
		}
		return -1;
	}
};