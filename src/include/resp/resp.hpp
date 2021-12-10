#pragma once

#include <cstdint>
#include <cstring>

#include "components.hpp"

namespace resp {
	// Status codes from the callbacks
	enum class status {
		OK	  = 0, // No error
		ERR	  = 1, // Generic error
		EIMPL = 2, // Not implemented
		ECMD  = 3, // Unknown command, only the parser should return this
	};

	// Callback type
	using cb_t = status (*)(char* data);

	// Key-value struct used for mapping handlers to resp commands
	struct rcmd_t {
		const char* name;
		resp::cb_t	cb;
	};

	/**
	 * @brief RESP command parser
	 */
	class parser {
	public:
		/**
		 * @brief Create a new RESP parser with _cmds
		 *
		 * @tparam N
		 * @param _cmds Array of commands
		 */
		template <size_t N>
		parser(const rcmd_t (&_cmds)[N]) {
			len = N;

			cmds = new rcmd_t[len];
			memcpy(cmds, &_cmds[0], sizeof(rcmd_t) * len);

			for (size_t i = 0; i < len; i++) {
				// a "command" named _INV is the callback for an invalid message
				if (strcmp(cmds[i].name, "_INV") == 0)
					cb_inv = cmds[i].cb;
				// a "command" named _ERR is the callback for an error message
				else if (strcmp(cmds[i].name, "_ERR") == 0)
					cb_err = cmds[i].cb;
			}
		}

		~parser();

		/**
		 * @brief Parse RESP messages into usable user-defined data structures
		 *
		 * @param data Pointer to the data to parse
		 * @return status - Handler error code
		 */
		status parse(char* data);

	private:
		cb_t cb_inv = NULL;
		cb_t cb_err = NULL;

		size_t	len	 = 0;
		rcmd_t* cmds = NULL;
	};
}; // namespace resp