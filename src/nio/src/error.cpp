#include "../error.hpp"

#include <string.h>

nio::error::error() {
}

nio::error::error(int _err) {
	set(_err);
}

void nio::error::set(int _err) {
	err = _err;
	msg = strerror(_err);
}

void nio::error::operator=(int _err) {
	set(_err);
}
