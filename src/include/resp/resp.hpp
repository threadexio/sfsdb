#pragma once

#include <cstdint>
#include <cstring>

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
	struct cb_lookup_t {
		const char* key;
		resp::cb_t	cb;
	};

	/**
	 * @brief Contains all the callbacks for the different commands. Leave null
	 * for unimplemented commands.
	 */
	struct callbacks {
		// Invalid command
		cb_t INV = NULL;
		// Error (-ERR message)
		cb_t ERR = NULL;
		// Get (+GET)
		cb_t GET = NULL;
		// Put (+PUT)
		cb_t PUT = NULL;
		// Delete (+DEL)
		cb_t DEL = NULL;
	};

	status parse(callbacks& cbs, char* data);
}; // namespace resp