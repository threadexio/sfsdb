#pragma once

#include <cstring>

#define RESP_COUNT(x) sizeof(x) / sizeof(x[0])

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
	status parse(const rcmd_t cmds[], size_t ncmds, char* data);
}; // namespace resp