#include "buffer.hpp"

#include <cstring>

nio::buffer::buffer() {};

nio::buffer::buffer(void* _data, size_t _len) {
	vec.resize(_len);
	memcpy(content(), _data, _len);
}

size_t nio::buffer::length() const {
	return vec.size();
}

bool nio::buffer::empty() const {
	return vec.empty();
}

void* nio::buffer::content() {
	return &vec[0];
}

void nio::buffer::clear() {
	vec.clear();
}

void nio::buffer::seek(size_t _new_pos) {
	pos = (_new_pos > length()) ? pos = length() : pos = _new_pos;
}

void nio::buffer::resize(size_t _len) {
	vec.resize(_len);
}