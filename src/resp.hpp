#pragma once

/*
#include <redis-cpp/resp/deserialization.h>
#include <redis-cpp/resp/serialization.h>
#include <redis-cpp/value.h>
*/
#include <redis-cpp/execute.h>

#include <cstring>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>

#define RESP_ERR_HANDLER "_ERR"
#define RESP_INV_HANDLER "_INV"

#define RESP_ERR_GENERIC	  -10000
#define RESP_ERR_NULL_HANDLER -10001 // Handler callback is nullptr
#define RESP_ERR_NO_HANDLER	  -10002 // No handler was found
#define RESP_ERR_BAD_REQ	  -10003 // Bad request

namespace resp {

	enum class types {
		ARRAY,
		BULK_STR,
		ERROR,
		NUL,
		INTEGER,
		SIMPLE_STR,
	};

	namespace resps	 = rediscpp::resp::serialization;
	namespace respds = rediscpp::resp::deserialization;

	using value = rediscpp::value;

	using cb_t = int (*)(std::stringstream &, std::stringstream &, void *);

	struct rcmd_t {
		const char *name;
		cb_t		cb;
	};

	inline types get_type(resp::value &val) {
		types ret;
		try {
			std::visit(
				rediscpp::resp::detail::overloaded {
					[&](respds::simple_string const &) {
						ret = types::SIMPLE_STR;
					},
					[&](respds::error_message const &) { ret = types::ERROR; },
					[&](respds::bulk_string const &) { ret = types::BULK_STR; },
					[&](respds::integer const &) { ret = types::INTEGER; },
					[&](respds::array const &) { ret = types::ARRAY; },
					[&](auto const &) { ret = types::NUL; }},
				val.get());
		} catch (const std::exception &) { return types::NUL; }
		return ret;
	}

	class parser {
	private:
		size_t	len	 = 0;
		rcmd_t *cmds = nullptr;

		cb_t cb_inv = nullptr;
		cb_t cb_err = nullptr;

	public:
		template <size_t N>
		parser(const rcmd_t (&_cmds)[N]) {
			len	 = N;
			cmds = new rcmd_t[N];
			memcpy(cmds, _cmds, N * sizeof(rcmd_t));

			for (size_t i = 0; i < len; i++) {
				// a "command" named _INV is the callback for an invalid command
				if (strcmp(cmds[i].name, RESP_INV_HANDLER) == 0)
					cb_inv = cmds[i].cb;
				// a "command" named _ERR is the callback for an error message
				else if (strcmp(cmds[i].name, RESP_ERR_HANDLER) == 0)
					cb_err = cmds[i].cb;
			}
		}

		~parser() {
			delete[] cmds;
		}

		/**
		 * @brief Dispatch the correct command handler. Command handler function
		 * signature: `int (*)(std::istringstream&)`.
		 *
		 * @param data Stream to parse
		 * @return int Handler return value, or RESP_ERR_* in case of parser
		 * error
		 */
		int parse(std::stringstream &req, std::stringstream &res, void *arg) {
			try {
				int ret = RESP_ERR_GENERIC;

				std::visit(rediscpp::resp::detail::overloaded {
							   [&](respds::simple_string const &val) -> void {
								   auto command = val.get();
								   for (size_t i = 0; i < len; i++) {
									   if (command == cmds[i].name) {
										   if (cmds[i].cb != nullptr) {
											   ret = cmds[i].cb(req, res, arg);
											   return;
										   } else {
											   ret = RESP_ERR_NULL_HANDLER;
											   return;
										   }
									   }
								   }
								   if (cb_inv != nullptr) {
									   ret = cb_inv(req, res, arg);
									   return;
								   } else {
									   ret = RESP_ERR_NULL_HANDLER;
									   return;
								   }
							   },
							   [&](respds::error_message const &) -> void {
								   if (cb_err != nullptr) {
									   ret = cb_err(req, res, arg);
									   return;
								   } else {
									   ret = RESP_ERR_NULL_HANDLER;
									   return;
								   }
							   },
							   [&](auto const &) {
								   if (cb_inv != nullptr) {
									   ret = cb_inv(req, res, arg);
									   return;
								   } else {
									   ret = RESP_ERR_NULL_HANDLER;
									   return;
								   }
							   }},
						   rediscpp::value {req}.get());
				return ret;
			} catch (const std::exception &e) { return RESP_ERR_BAD_REQ; }
		}
	};

} // namespace resp