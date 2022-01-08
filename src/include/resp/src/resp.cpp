#include "resp.hpp"

#include "types.hpp"

#define RESP_CALL_IFN_NULL(x, y, z)                                            \
	{                                                                          \
		if (x == NULL)                                                         \
			return 0;                                                          \
		else                                                                   \
			return x(y, z);                                                    \
	}

resp::parser::~parser() {
	delete[] cmds;
}

int resp::parser::parse(char* data, void* arg) {
	if (*data != '+' && *data != '-') { // if the request doesnt begin with
										// a - or + then it is not valid
		RESP_CALL_IFN_NULL(cb_inv, data, arg);
	} else if (*data == '-') {
		RESP_CALL_IFN_NULL(cb_err, data, arg);
	}

	types::simstr command(data);
	for (size_t i = 0; i < len; i++)
		if (strcmp(cmds[i].name, command.value) == 0)
			RESP_CALL_IFN_NULL(cmds[i].cb, data, arg);

	RESP_CALL_IFN_NULL(cb_inv, data, arg);
}