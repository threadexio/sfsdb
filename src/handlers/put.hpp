#pragma once
#include "helper.hpp"

//=========//

extern volume::volume_type vol;

namespace handlers {
	REGISTER_HANDLER(put) {
		/**
		 * Command format:
		 *
		 * PUT [file name] [file data]
		 *
		 * Binary format:
		 *
		 * Request: [header] (string)[fname] (bigdata)[fdata]
		 * Response: [header] (string)[fid]
		 */
		UNUSED(head);
		UNUSED(arg);

		plog::v(LOG_INFO "parser", "PUT command");

		protocol::types::string fname;
		Error					err;
		if (auto err = protocol::get_type(req, fname)) {
			plog::v("put", err.msg);
			protocol::messages::error(err.msg).to(res);
			return HANDLER_ERROR;
		}
		if (fname.str == "") {
			protocol::messages::error("Expected file name").to(res);
			return HANDLER_ERROR;
		}

		protocol::types::bigdata fdata;
		if (auto err = protocol::get_type(req, fdata)) {
			protocol::messages::error("Wrong parameter type").to(res);
			return HANDLER_ERROR;
		}

		protocol::types::string fid;
		if (auto r = vol.store(fname.str, req, fdata.length)) {
			plog::v(LOG_ERROR "fs", "Cannot store file: %s", r.Err().msg);
			protocol::messages::error(r.Err().msg).to(res);
			return HANDLER_ERROR;
		} else {
			fid.str = r.Ok();
		}

		// Send response
		protocol::types::header(
			protocol::status::SUCCESS,
			protocol::types::string::HEADER_SIZE + fid.length)
			.to(res);
		fid.to(res);

		return HANDLER_SUCCESS;
	}
} // namespace handlers