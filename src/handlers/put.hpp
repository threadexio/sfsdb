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

		plog::v(LOG_INFO "parser", "PUT command");

		auto* stream = (stream_type*)arg;

		protocol::types::string fname;
		if (protocol::get_type(req) != protocol::types::ids::STRING) {
			plog::v("put", "Wrong parameter type");
			protocol::messages::error("Wrong parameter type").to(res);
			return HANDLER_ERROR;
		} else {
			fname.from(req);
			if (fname.str == "") {
				protocol::messages::error("Expected file name").to(res);
				return HANDLER_ERROR;
			}
		}

		protocol::types::bigdata fdata;
		if (protocol::get_type(req) != protocol::types::ids::BIGDATA) {
			protocol::messages::error("Wrong parameter type").to(res);
			return HANDLER_ERROR;
		}
		fdata.from(req);

		protocol::types::string fid;
		if (auto r = vol.get_name(fname.str).create()) {
			plog::v(LOG_ERROR "fs", "Cannot create file: %s", r.Err().msg);
			protocol::messages::error(r.Err().msg).to(res);
			return HANDLER_ERROR;
		} else {
			fid.str	   = r.Ok();
			fid.length = fid.str.length();
		}

		storage::data_type fobj;
		if (auto r = vol.get_id(fid.str)) {
			plog::v(LOG_ERROR "fs",
					"Cannot find %s: %s",
					fid.str.c_str(),
					r.Err().msg);
			protocol::messages::error(r.Err().msg).to(res);
			return HANDLER_ERROR;
		} else {
			fobj = r.Ok();
		}

		fobj.save(req, fdata.length);

		// Send response
		{
			std::stringstream tmp;

			protocol::types::header(
				protocol::status::SUCCESS,
				protocol::types::string::DATA_HEADER_SIZE + fid.length)
				.to(tmp);
			fid.to(tmp);

			if (auto r = stream->write(tmp.str().c_str(), tmp.str().length()))
				return HANDLER_NO_SEND_RES;
		}

		return HANDLER_SUCCESS;
	}
} // namespace handlers