#pragma once
#include "helper.hpp"

//=========//

#include <fcntl.h>
#include <sys/mman.h>

extern volume::volume_type vol;

namespace handlers {
	REGISTER_HANDLER(get)
	try {
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

		auto* stream = (stream_type*)arg;

		protocol::types::string fid;
		if (auto err = protocol::get_type(req, fid)) {
			protocol::messages::error(err.msg).to(res);
			return HANDLER_ERROR;
		}
		if (fid.str == "") {
			protocol::messages::error("Expected file id").to(res);
			return HANDLER_ERROR;
		}

		// Get file data object
		storage::object file;
		if (auto r = vol.get_id(fid.str)) {
			protocol::messages::error(r.Err().msg).to(res);
			return HANDLER_ERROR;
		} else
			file = r.Ok();

		// Get file size
		uint32_t fsize;
		if (auto r = file.details()) {
			plog::v(LOG_WARNING "fs", "stat: %s", r.Err().msg);
			protocol::messages::error(r.Err().msg).to(res);
			return HANDLER_ERROR;
		} else
			fsize = r.Ok().size;

		// Send the file data to the client
		int fd = open(file.data_path.c_str(), O_RDONLY);
		if (fd < 0) {
			plog::v(LOG_WARNING "fs", "open: %s", Error(errno).msg);
			protocol::messages::error(Error(errno).msg).to(res);
			return HANDLER_ERROR;
		}

		// mmap() the file into memory
		void* fptr =
			mmap(NULL, fsize, PROT_READ, MAP_PRIVATE | MAP_STACK, fd, 0);
		if (fptr == MAP_FAILED) {
			protocol::messages::error(Error(errno).msg).to(res);
			return HANDLER_ERROR;
		}

		{ // Send the headers
			std::stringstream tmp;

			protocol::types::header(
				0, protocol::types::bigdata::HEADER_SIZE + fsize)
				.to(tmp);

			protocol::types::bigdata(fsize).to(tmp);

			stream->write(tmp.str().c_str(), tmp.str().length(), MSG_MORE);
		}

		// Send the file
		stream->write((const char*)fptr, fsize, MSG_MORE);

		// Send the magic byte
		stream->write(&protocol::MAGIC, 1);

		munmap(fptr, fsize);
		close(fd);

		return HANDLER_NO_SEND_RES;

	} catch (const nio::io_error& e) {
		plog::v(LOG_WARNING "net", "%s: %s", e.which(), e.what());
		return HANDLER_NO_SEND_RES;
	}
} // namespace handlers