#pragma once
#include "helper.hpp"

//=========//

extern volume::volume_type vol;

namespace handlers {
	REGISTER_HANDLER(desc) {
		/**
		 * Command format:
		 *
		 * DESC [file id]
		 *
		 * Binary format:
		 *
		 * Request: [header] (string)[fid]
		 * Response: [header] (string)[filename] (integer)[filesize]
		 */
		UNUSED(head);
		UNUSED(arg);

		plog::v(LOG_INFO "parser", "DESC command");

		protocol::types::string fid;
		if (auto err = protocol::get_type(req, fid)) {
			plog::v(LOG_INFO "desc", err.msg);
			protocol::messages::error(err.msg).to(res);
			return HANDLER_ERROR;
		}
		if (fid.str == "") {
			protocol::messages::error("Expected file id").to(res);
			return HANDLER_ERROR;
		}

		storage::data_type fobj;
		if (auto r = vol.get_id(fid.str)) {
			protocol::messages::error(r.Err().msg).to(res);
			return HANDLER_ERROR;
		} else {
			fobj = r.Ok();
		}

		storage::meta mdata;
		if (auto r = fobj.details()) {
			plog::v(LOG_ERROR "fs",
					"Cannot stat %s: %s",
					fid.str.c_str(),
					r.Err().msg);
			protocol::messages::error(r.Err().msg).to(res);
			return HANDLER_ERROR;
		} else {
			mdata = r.Ok();
		}

		std::string fname;
		if (auto r = vol.get_mapping(fid.str)) {
			protocol::messages::error(r.Err().msg).to(res);
			return HANDLER_ERROR;
		} else {
			fname = r.Ok().name;
		}

		{ // Send the response
			std::stringstream tmp;

			protocol::types::string(fname.c_str()).to(tmp);
			protocol::types::integer(mdata.size).to(tmp);

			protocol::types::header(protocol::status::SUCCESS,
									tmp.str().length())
				.to(res);
			res << tmp.str();
		}
		return HANDLER_SUCCESS;
	}
} // namespace handlers