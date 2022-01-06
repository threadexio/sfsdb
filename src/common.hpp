#pragma once

#include <cstring>

/**
 * @brief Simple type for error handling in return values. ("Inspired" by Rust)
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

	[[nodiscard]] inline _T& Ok() {
		return ok;
	}

	[[nodiscard]] inline _E& Err() {
		return err;
	}

	[[nodiscard]] inline Result<_T, _E>& Ok(const _T& _ok) {
		_fail = false;
		ok	  = _ok;
		return *this;
	}

	[[nodiscard]] inline Result<_T, _E>& Err(const _E& _err) {
		_fail = true;
		err	  = _err;
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

	Error() = default;

	Error(int _errno) : no(_errno), msg(strerror(no)) {
	}

	inline operator bool() {
		return no == 0;
	}
};