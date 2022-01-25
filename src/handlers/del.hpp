#pragma once
#include "helper.hpp"

//=========//

extern volume::volume_type vol;

namespace handlers {
	REGISTER_HANDLER(del) {
		/**
		 * Command format:
		 *
		 * DEL [file id]
		 *
		 * Binary format:
		 *
		 * Request: [header] (string)[fid]
		 * Response: [header] {error}
		 */
		UNUSED(head);
		UNUSED(arg);

		plog::v(LOG_INFO "parser", "DEL command");

		protocol::types::string fid;
		if (protocol::get_type(req) != protocol::types::ids::STRING) {
			plog::v(LOG_INFO "del", "Wrong parameter type");
			protocol::messages::error("Wrong parameter type").to(res);
			return HANDLER_ERROR;
		} else {
			fid.from(req);
			if (fid.str == "") {
				protocol::messages::error("Expected file id").to(res);
				return HANDLER_ERROR;
			}
		}

		if (auto r = vol.remove(fid.str)) {
			plog::v(LOG_ERROR "fs", "Cannot remove file: %s", r.Err().msg);
			protocol::messages::error(r.Err().msg).to(res);
			return HANDLER_ERROR;
		}

		// Send the headers
		protocol::types::header(protocol::status::SUCCESS, 0).to(res);
		return HANDLER_SUCCESS;
	}
} // namespace handlers