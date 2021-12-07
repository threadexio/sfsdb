#include "components.hpp"

resp::components::integer::integer(char*& _data) {
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

resp::components::string::string(char*& _data) {
	if (*_data == '+') { // simple string
		_data++;
		auto* tmp = _data;
		while (*tmp != '\r') {
			length++;
			tmp++;
		}
	} else if (*_data == '$') { // bulk string, can also be used for
								// binary data
		while (*_data != '\r') {
			length = (length * 10) + (*_data - '0');
			_data++;
		}
		_data += 2;

		if (length == -1) // null values (value == NULL)
			return;
	} else
		return;

	value = new char[length + 1];

	// Null terminator
	value[length] = 0;

	std::memcpy(value, _data, length);
	_data += length + 2;
}

resp::components::string::~string() {
	delete[] value;
}

resp::components::error::error(char*& _data) {
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

resp::components::error::~error() {
	delete[] value;
}