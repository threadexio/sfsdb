#include "types.hpp"

#include <cstdio>
#include <cstring>

resp::types::integer::integer(int64_t _value) {
	value = _value;
}

resp::types::integer::integer(char*& _data) {
	if (*_data != ':')
		return;

	_data++;

	int sign = 1;
	if (*_data == '-') {
		sign = -1;
		_data++;
	}

	while (*_data != '\r') {
		value = (value * 10) + (*_data - '0');
		_data++;
	}
	value *= sign;

	_data += 2;
}

size_t resp::types::integer::serialize(char* _data) const {
	return sprintf(_data, ":%ld\r\n", value);
}

resp::types::bulkstr::bulkstr(const char* _data) {
	length = strlen(_data);
	value  = new char[length + 1];
	std::memcpy(value, _data, length);
}

resp::types::bulkstr::bulkstr(char*& _data) {
	if (*_data != '$')
		return;

	_data++;
	while (*_data != '\r') {
		length = (length * 10) + (*_data - '0');
		_data++;
	}
	_data += 2;

	value = new char[length + 1];

	// Null terminator
	value[length] = 0;

	std::memcpy(value, _data, length);
	_data += length + 2;
}

resp::types::bulkstr::~bulkstr() {
	delete[] value;
}

size_t resp::types::bulkstr::serialize(char* _data) const {
	return sprintf(_data, "$%ld\r\n%s\r\n", length, value);
}

resp::types::simstr::simstr(const char* _data) {
	length = strlen(_data);
	value  = new char[length + 1];
	std::memcpy(value, _data, length);
}

resp::types::simstr::simstr(char*& _data) {
	if (*_data != '+')
		return;

	_data++;
	auto* tmp = _data;
	while (*tmp != '\r') {
		length++;
		tmp++;
	}

	value = new char[length + 1];

	// Null terminator
	value[length] = 0;

	std::memcpy(value, _data, length);
	_data += length + 2;
}

resp::types::simstr::~simstr() {
	delete[] value;
}

size_t resp::types::simstr::serialize(char* _data) const {
	return sprintf(_data, "+%s\r\n", value);
}

resp::types::error::error(const char* _data) {
	length = strlen(_data);
	value  = new char[length + 1];
	std::memcpy(value, _data, length);
}

resp::types::error::error(char*& _data) {
	if (*_data != '-')
		return;

	_data++;
	auto* tmp = _data;
	while (*tmp != '\r') {
		length++;
		tmp++;
	}

	value = new char[length + 1];

	value[length] = 0;

	std::memcpy(value, _data, length);
	_data += length + 2;
}

resp::types::error::~error() {
	delete[] value;
}

size_t resp::types::error::serialize(char* _data) const {
	return sprintf(_data, "-%s\r\n", value);
}