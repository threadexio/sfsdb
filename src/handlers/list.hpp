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

		plog::v(LOG_INFO "parser", "LIST command");

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
			std::stringstream tmp;
			auto			  fobj = vol.get_name(fname.str);

			protocol::types::smallint(fobj.ids.size()).to(tmp);
			for (auto &id : fobj.ids) {
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