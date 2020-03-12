#pragma once

#include <vector>

namespace utils {
#define ORIGINAL_STREAM
#ifdef ORIGINAL_STREAM
	template<typename T>
	class Stream {
		using it = typename std::vector<T>::const_iterator;

		it _end;
		it _pos;
		it _saved_pos;
	public:
		Stream(const std::vector<T>& source) : Stream(source.begin(), source.end()) {}
		Stream(it begin, it end) : _pos(begin), _saved_pos(begin), _end(end) {}
		bool isEnd() const { return _pos == _end; }
		bool is(T t) const { return !isEnd() and (*_pos == t); }
		T peak() const { return *_pos; }
		T peakAndNext() { return *_pos++; }
		void next() { ++_pos; }
		void prev() { --_pos; }
		it pos() const { return _pos; }
		void save() { _saved_pos = _pos; }
		void rewind() { _pos = _saved_pos; }

		const T* operator->() const { return &(*_pos); }
	};
#else
	template<typename T>
	class Stream {
		using it = const T*;

		it _end;
		it _pos;
		it _saved_pos;
	public:
		template<class Container>
		Stream(const Container& source) : Stream(source.begin(), source.end()) {}

		template<typename Iterator>
		Stream(Iterator begin, Iterator end) : _pos(&(*begin)), _saved_pos(&(*begin)), _end(&(*end)) {}

		bool isEnd() const { return _pos == _end; }
		bool is(T t) const { return !isEnd() and (*_pos == t); }
		T peak() const { return *_pos; }
		T peakAndNext() { return *_pos++; }
		void next() { ++_pos; }
		void prev() { --_pos; }
		it pos() const { return _pos; }
		void save() { _saved_pos = _pos; }
		void rewind() { _pos = _saved_pos; }

		const T* operator->() const { return _pos; }
	};
#endif // !1
}