#include "resp.hpp"

#include "types.hpp"

#define RESP_CALL_IFN_NULL(x, d)                                               \
	{                                                                          \
		if (x == NULL)                                                         \
			return 0;                                                          \
		else                                                                   \
			return x(d);                                                       \
	}

resp::parser::~parser() {
	delete[] cmds;
}

int resp::parser::parse(char *data) {
	if (*data != '+' && *data != '-') { // if the request doesnt begin with
										// a - or + then it is not valid
		RESP_CALL_IFN_NULL(cb_inv, data);
	} else if (*data == '-') {
		RESP_CALL_IFN_NULL(cb_err, data);
	}

	types::simstr command(data);
	for (size_t i = 0; i < len; i++)
		if (strcmp(cmds[i].name, command.value) == 0)
			RESP_CALL_IFN_NULL(cmds[i].cb, data);

	RESP_CALL_IFN_NULL(cb_inv, data);
}