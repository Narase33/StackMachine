#pragma once

#include "BasicType.h"

namespace base {
	enum class ObjectType {
		Array
	};

	struct Object {
		Object(ObjectType type) :
			type(type) {
		}

		virtual ~Object() = default;

		const ObjectType type;
	};

	struct Array : public Object {

		size_t size() const {
			return _size;
		}

		TypeIndex elementType() const {
			return static_cast<base::TypeIndex>(values.index());
		}

		static Array* create(size_t size, TypeIndex type) {
			char* buffer = new char[sizeof(Array) + (sizeOfType(type) * size)];
			Array* arrayClass = new (buffer) Array(size);

			switch (type) {
				case TypeIndex::Int:
				{
					sm_int* dataArray = new (buffer + sizeof(Array)) sm_int[size];
					arrayClass->values = dataArray;
				}
				break;
				case TypeIndex::Uint:
				{
					sm_uint* dataArray = new (buffer + sizeof(Array)) sm_uint[size];
					arrayClass->values = dataArray;
				}
				break;
				case TypeIndex::Float:
				{
					sm_float* dataArray = new (buffer + sizeof(Array)) sm_float[size];
					arrayClass->values = dataArray;
				}
				break;
				case TypeIndex::Bool:
				{
					sm_bool* dataArray = new (buffer + sizeof(Array)) sm_bool[size];
					arrayClass->values = dataArray;
				}
				break;
			}

			return arrayClass;
		}

		static void free(Array* arr) {
			char* buffer = reinterpret_cast<char*>(arr);
			arr->~Array();
			delete(buffer);
		}

		virtual ~Array() = default;

	protected:
		Array(size_t size) :
			Object(ObjectType::Array), _size(size) {
		}

	private:
		const size_t _size;
		std::variant<sm_int*, sm_uint*, sm_float*, sm_bool*> values;
	};
}