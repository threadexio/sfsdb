#pragma once

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

public:
	_T ok;
	_E err;

	Result() = default;

	[[nodiscard]] inline Result<_T, _E>& Ok(_T _ok) {
		_fail = false;
		ok	  = _ok;
		return *this;
	}

	[[nodiscard]] inline Result<_T, _E>& Err(_E _err) {
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