#pragma once
#include "handlers.hpp"

//=========//

#include "log.hpp"

namespace handlers {
	REGISTER_HANDLER(invalid) {
		UNUSED(res);
		UNUSED(req);
		UNUSED(arg);

		plog::v("invalid", "Invalid command: %x", head.type);
		return 0;
	}
} // namespace handlers