#pragma once

#include <cstring>

#include "components.hpp"

#define RESP_COUNT(x) sizeof(x) / sizeof(x[0])

#define RESP_CALL_IFN_NULL(x, d)                                               \
	{                                                                          \
		if (x == NULL)                                                         \
			return resp::status::NIMPL;                                        \
		else                                                                   \
			return x(d);                                                       \
	}

namespace resp {
	// Internal status codes from the function handlers
	enum class status {
		OK	  = 0, // No error
		ERR	  = 1, // Generic error
		NIMPL = 2, // Not implemented
		NCMD  = 3, // Unknown command
	};

	// Callback type
	using cb_t = status (*)(char* data);

	// Key-value struct used for internally mapping functions to resp commands
	struct rcmd_t {
		const char* name;
		resp::cb_t	cb;
	};

	/**
	 * @brief Parse RESP messages into usable user-defined data structures
	 *
	 * @param cmds Array of commands
	 * @param ncmds Length of cmds
	 * @param data Pointer to the data to parse
	 * @return status - Handler error code
	 */
	template <size_t N>
	status parse(const rcmd_t (&cmds)[N], char* data) {
		size_t ncmds = N;

		cb_t cb_inv = NULL;
		cb_t cb_err = NULL;

		for (size_t i = 0; i < ncmds; i++) {
			// a "command" named _INV is the callback for an invalid message
			if (strcmp(cmds[i].name, "_INV") == 0)
				cb_inv = cmds[i].cb;
			// a "command" named _ERR is the callback for an error message
			else if (strcmp(cmds[i].name, "_ERR") == 0)
				cb_err = cmds[i].cb;
		}

		if (*data != '+' && *data != '-') { // if the request doesnt begin with
											// a - or + then it is not valid
			RESP_CALL_IFN_NULL(cb_inv, data);
		} else if (*data == '-') {
			RESP_CALL_IFN_NULL(cb_err, data);
		}

		components::string command(data);
		for (size_t i = 0; i < ncmds; i++)
			if (strcmp(cmds[i].name, command.value) == 0)
				RESP_CALL_IFN_NULL(cmds[i].cb, data);

		return status::NCMD;
	}
}; // namespace resp