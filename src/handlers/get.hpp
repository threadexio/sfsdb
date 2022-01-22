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
		 * Request: [header][fid]
		 * Response: [header][filedata]
		 */
		UNUSED(head);

		plog::v(LOG_INFO "parser", "GET command");

		auto* stream = (stream_type*)arg;

		uid::uid_type fid(req, 32);

		storage::data_type file;
		if (auto r = vol.get_id(fid)) {
			protocol::types::error(r.Err().msg).to(res);
			return HANDLER_ERROR;
		} else
			file = r.Ok();

		uint32_t fsize;
		if (auto r = file.details()) {
			plog::v(
				LOG_WARNING "fs", "Cannot get file details: %s", r.Err().msg);
			protocol::types::error(r.Err().msg).to(res);
			return HANDLER_ERROR;
		} else
			fsize = r.Ok().size;

		// Send the file data to the client
		// mmap() the file into memory
		int fd = open(file.path().c_str(), O_RDONLY);
		if (fd < 0) {
			plog::v(LOG_ERROR "fs", "Cannot open file: %s", Error(errno).msg);
			protocol::types::error(Error(errno).msg).to(res);
			return HANDLER_ERROR;
		}

		void* fptr =
			mmap(NULL, fsize, PROT_READ, MAP_PRIVATE | MAP_STACK, fd, 0);
		if (fptr == MAP_FAILED) {
			plog::v(LOG_ERROR "fs", "Cannot map file: %s", Error(errno).msg);
			protocol::types::error(Error(errno).msg).to(res);
			return HANDLER_ERROR;
		}

		{ // Send the header
			char tmp[protocol::HEADER_SIZE];

			protocol::header {protocol::SUCCESS_HANDLER, fsize}.to(tmp);

			if (auto r = stream->write(tmp, protocol::HEADER_SIZE, MSG_MORE)) {}
		}

		if (auto r = stream->write((const char*)fptr, fsize)) {}

		munmap(fptr, fsize);
		close(fd);

		return HANDLER_SUCCESS;
	}
} // namespace handlers