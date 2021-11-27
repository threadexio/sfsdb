#include "resp.hpp"

#include "components.hpp"

#define RESP_CALL_IFN_NULL(x, d)                                               \
	{                                                                          \
		if (x == NULL)                                                         \
			return resp::status::NIMPL;                                        \
		else                                                                   \
			return x(d);                                                       \
	}

resp::status resp::parse(callbacks& cbs, char* data) {
	if (*data != '+' && *data != '-') { // if the request doesnt begin with a -
										// or + then it is not valid
		RESP_CALL_IFN_NULL(cbs.INV, data);
	} else if (*data == '-') {
		RESP_CALL_IFN_NULL(cbs.ERR, data);
	}

	cb_lookup_t lookup_table[] = {
		{"GET", cbs.GET},
		{"PUT", cbs.PUT},
		{"DEL", cbs.DEL},
	};

	components::string command(data);

	size_t nkeys = sizeof(lookup_table) / sizeof(lookup_table[0]);
	for (size_t i = 0; i < nkeys; i++) {
		cb_lookup_t* t = &lookup_table[i];
		if (strcmp(t->key, command.value) == 0) {
			RESP_CALL_IFN_NULL(t->cb, data);
		}
	}

	return status::NCMD;
}