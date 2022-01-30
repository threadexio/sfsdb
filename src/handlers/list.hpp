#pragma once
#include "helper.hpp"

//=========//

extern volume::volume_type vol;

namespace handlers {
	REGISTER_HANDLER(list) {
		/**
		 * Command format:
		 *
		 * LIST [file name]
		 *
		 * Binary format:
		 *
		 * Request: [header] (string)[filename]
		 * Response: [header] (smallint)[resultnum] (string)[fid] ...
		 *
		 */
		UNUSED(head);
		UNUSED(arg);

		protocol::types::string fname;
		if (auto err = protocol::get_type(req, fname)) {
			protocol::messages::error(err.msg).to(res);
			return HANDLER_ERROR;
		}

		if (fname.str == "") {
			protocol::messages::error("Expected file name").to(res);
			return HANDLER_ERROR;
		}

		{ // Send the response
			std::stringstream		   tmp;
			std::vector<uid::uid_type> ids;

			if (auto r = vol.get_name(fname.str)) {
				protocol::messages::error(r.Err().msg).to(res);
				return HANDLER_ERROR;
			} else
				ids = r.Ok();

			protocol::types::smallint(ids.size()).to(tmp);
			for (auto &id : ids) {
				protocol::types::string(id.c_str()).to(tmp);
			}

			protocol::types::header(protocol::status::SUCCESS,
									tmp.str().length())
				.to(res);
			res << tmp.str();
		}
		return HANDLER_SUCCESS;
	}
} // namespace handlers