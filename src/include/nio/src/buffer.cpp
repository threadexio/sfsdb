#include "buffer.hpp"

#include <cstring>

nio::buffer::buffer(size_t _len) {
	resize(_len);
};

nio::buffer::buffer(void* _data, size_t _len) {
	vec.resize(_len);
	memcpy(data(), _data, _len);
}

uint8_t& nio::buffer::at(size_t _index) {
	if (_index >= length()) {
		_index = length() - 1;
	}

	return *(vec.data() + _index);
}

size_t nio::buffer::length() const {
	return vec.size();
}

bool nio::buffer::empty() const {
	return vec.empty();
}

const void* nio::buffer::data() const {
	return &vec[0];
}

void* nio::buffer::data() {
	return &vec[0];
}

void nio::buffer::clear() {
	vec.clear();
}

void nio::buffer::seek(size_t _new_pos) {
	// This throws a compiler warning so we have to do a normal if here
	// pos = (_new_pos > length()) ? pos = length() : pos = _new_pos;

	if (_new_pos > length()) {
		pos = length();
	} else {
		pos = _new_pos;
	}
}

void nio::buffer::resize(size_t _len) {
	vec.resize(_len);
}

uint8_t& nio::buffer::operator[](size_t _index) {
	return at(_index);
}

nio::buffer::operator char*() {
	return (char*)data();
}