#include "resp.hpp"

#include "components.hpp"

#define RESP_CHECK_NULL(x)                                                     \
	{                                                                          \
		if (x == NULL)                                                         \
			return resp::status::NIMPL;                                        \
	}

resp::status resp::parse(callbacks& cbs, char* data) {
	if (*data != '+' && *data != '-') { // upon invalid command, the first
										// callback from cbs will be executed
		RESP_CHECK_NULL(cbs.INV);
		return cbs.INV(data);
	}

	cb_lookup_t lookup_table[] = {
		{"ERR", cbs.ERR},
		{"GET", cbs.GET},
		{"PUT", cbs.PUT},
		{"DEL", cbs.DEL},
	};

	components::string command(data);

	size_t nkeys = sizeof(lookup_table) / sizeof(lookup_table[0]);
	for (size_t i = 0; i < nkeys; i++) {
		cb_lookup_t* t = &lookup_table[i];
		if (strcmp(t->key, command.value) == 0) {
			RESP_CHECK_NULL(t->cb);
			return t->cb(data);
		}
	}

	return status::NCMD;
}