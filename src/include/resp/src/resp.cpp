
#include "resp.hpp"

#include <assert.h>

#include <cstring>

#include "components.hpp"

#define RESP_CALL_IFN_NULL(x, d)                                               \
	{                                                                          \
		if (x == NULL)                                                         \
			return resp::status::NIMPL;                                        \
		else                                                                   \
			return x(d);                                                       \
	}

resp::status resp::parse(const rcmd_t cmds[], size_t ncmds, char* data) {
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