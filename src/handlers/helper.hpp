#pragma once

#include <string>
#include <string_view>

#include "common.hpp"
#include "log.hpp"
#include "misc.hpp"
#include "nio/base/stream.hpp"
#include "resp.hpp"

namespace helper {

	inline auto get_value(char* data) {
		std::stringstream tmp {data};
		return resp::value {tmp};
	}

	inline auto get_simple_str(std::stringstream& tmp) {
		Result<std::string, const char*> ret;

		try {
			auto val = resp::value {tmp};
			if (val.is_simple_string())
				return ret.Ok(std::string(val.as_simple_string()));
			else
				throw std::invalid_argument("Invalid argument");
		} catch (std::exception& e) { return ret.Err(e.what()); }
	}

	inline auto get_bulk_str(std::stringstream& tmp) {
		Result<std::string, const char*> ret;

		try {
			auto val = resp::value {tmp};
			if (val.is_bulk_string())
				return ret.Ok(std::string(val.as_bulk_string()));
			else
				throw std::invalid_argument("Invalid argument");
		} catch (std::exception& e) { return ret.Err(e.what()); }
	}

	inline auto get_integer(std::stringstream& tmp) {
		Result<int64_t, const char*> ret;

		try {
			auto val = resp::value {tmp};
			if (val.is_integer())
				return ret.Ok(val.as_integer());
			else
				throw std::invalid_argument("Invalid argument");
		} catch (std::exception& e) { return ret.Err(e.what()); }
	}

	inline auto get_error(std::stringstream& tmp) {
		Result<std::string, const char*> ret;

		try {
			auto val = resp::value {tmp};
			if (val.is_integer())
				return ret.Ok(std::string(val.as_error_message()));
			else
				throw std::invalid_argument("Invalid argument");
		} catch (std::exception& e) { return ret.Err(e.what()); }
	}

	inline int log_read_error(const nio::Error& e) {
		plog::v(LOG_WARNING "net", std::string("Cannot read: ") + e.msg);
		return HANDLER_NO_SEND_RES;
	}

	inline int log_write_error(const nio::Error& e) {
		plog::v(LOG_WARNING "net", std::string("Cannot write: ") + e.msg);
		return HANDLER_NO_SEND_RES;
	}

} // namespace helper