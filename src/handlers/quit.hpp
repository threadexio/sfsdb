#pragma once
#include "helper.hpp"

//=========//

extern volume::volume_type vol;

namespace handlers {
	REGISTER_HANDLER(quit) {
		/**
		 * Command format:
		 *
		 * QUIT [file name]
		 *
		 * Binary format:
		 *
		 * Request: [header] (string)[filename]
		 * Response: [header] (smallint)[resultnum] (string)[fid] ...
		 *
		 */
		UNUSED(head);
		UNUSED(arg);
		UNUSED(req);
		UNUSED(res);

		auto* stream = (stream_type*)arg;

		stream->shutdown();

		return HANDLER_NO_SEND_RES;
	}
} // namespace handlers