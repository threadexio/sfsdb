#pragma once
#include "helper.hpp"

//=========//

namespace handlers {
	REGISTER_HANDLER(invalid) {
		UNUSED(head);
		UNUSED(res);
		UNUSED(req);
		UNUSED(arg);

		return HANDLER_NO_SEND_RES;
	}
} // namespace handlers