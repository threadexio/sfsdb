#pragma once

#include <algorithm>
#include <cstring>

#define UNUSED(x) (void)x

/**
 * @brief Simple type for error handling in return values. ("Inspired" by
 * Rust)
 *
 * @tparam _T Type of the value
 * @tparam _E Type of the error
 */
template <typename _T, typename _E>
class Result {
private:
	bool _fail;

	_T ok;
	_E err;

public:
	Result() {};

	[[nodiscard]] inline _T&& Ok() {
		return std::move(ok);
	}

	[[nodiscard]] inline _E&& Err() {
		return std::move(err);
	}

	[[nodiscard]] inline Result<_T, _E>& Ok(_T&& _ok) {
		_fail = false;
		ok	  = std::move(_ok);
		return *this;
	}

	[[nodiscard]] inline Result<_T, _E>& Err(_E&& _err) {
		_fail = true;
		err	  = std::move(_err);
		return *this;
	}

	inline bool is_ok() const {
		return ! _fail;
	}

	inline bool is_err() const {
		return _fail;
	}

	inline operator bool() {
		return is_err();
	}
};

struct Error {
	int			no;
	const char* msg;

	Error() {};

	Error(int _errno) : no(_errno), msg(strerror(no)) {
	}

	Error(int _errno, const char* _msg) : no(_errno), msg(_msg) {
	}

	Error(Error&& other) noexcept {
		no	= other.no;
		msg = other.msg;
	}

	Error& operator=(Error&& other) noexcept {
		if (this == &other)
			return *this;

		no	= other.no;
		msg = other.msg;
		return *this;
	}

	inline operator bool() {
		return no != 0;
	}
};