#pragma once
#include "helper.hpp"

//=========//

#include <fcntl.h>
#include <sys/mman.h>

extern volume::volume_type vol;

namespace handlers {
	REGISTER_HANDLER(get) {
		/**
		 * Command format:
		 *
		 * GET [file id]
		 *
		 * Binary format:
		 *
		 * Request: [header] (string)[fid]
		 * Response: [header] (bigdata)[filedata]
		 */
		UNUSED(head);

		plog::v(LOG_INFO "parser", "GET command");

		auto* stream = (stream_type*)arg;

		protocol::types::string fid;
		if (auto err = protocol::get_type(req, fid)) {
			plog::v(LOG_INFO "get", err.msg);
			protocol::messages::error(err.msg).to(res);
			return HANDLER_ERROR;
		}
		if (fid.str == "") {
			protocol::messages::error("Expected file id").to(res);
			return HANDLER_ERROR;
		}

		// Get file data object
		storage::data_type file;
		if (auto r = vol.get_id(fid.str)) {
			protocol::messages::error(r.Err().msg).to(res);
			return HANDLER_ERROR;
		} else
			file = r.Ok();

		// Get file size
		uint32_t fsize;
		if (auto r = file.details()) {
			plog::v(
				LOG_WARNING "fs", "Cannot get file details: %s", r.Err().msg);
			protocol::messages::error(r.Err().msg).to(res);
			return HANDLER_ERROR;
		} else
			fsize = r.Ok().size;

		// Send the file data to the client
		int fd = open(file.path().c_str(), O_RDONLY);
		if (fd < 0) {
			plog::v(LOG_ERROR "fs", "Cannot open file: %s", Error(errno).msg);
			protocol::messages::error(Error(errno).msg).to(res);
			return HANDLER_ERROR;
		}

		// mmap() the file into memory
		void* fptr =
			mmap(NULL, fsize, PROT_READ, MAP_PRIVATE | MAP_STACK, fd, 0);
		if (fptr == MAP_FAILED) {
			plog::v(LOG_ERROR "fs", "Cannot map file: %s", Error(errno).msg);
			protocol::messages::error(Error(errno).msg).to(res);
			return HANDLER_ERROR;
		}

		{ // Send the headers
			std::stringstream tmp;

			protocol::types::header(
				0, protocol::types::bigdata::HEADER_SIZE + fsize)
				.to(tmp);

			protocol::types::bigdata(fsize).to(tmp);

			if (auto r = stream->write(
					tmp.str().c_str(), tmp.str().length(), MSG_MORE))
				return HANDLER_NO_SEND_RES;
		}

		// Send the file
		if (auto r = stream->write((const char*)fptr, fsize))
			return HANDLER_NO_SEND_RES;

		munmap(fptr, fsize);
		close(fd);

		return HANDLER_NO_SEND_RES;
	}
} // namespace handlers