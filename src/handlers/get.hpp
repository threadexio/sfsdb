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

		/*
		// Parse command arguments
		uid::uid_type fid;
		if (auto r = helper::get_simple_str(req)) {
			plog::v("parser", "%s", r.Err());
			return HANDLER_ERROR;
		} else
			fid = r.Ok();

		// Find requested file
		storage::data_type file;
		if (auto r = vol.get_id(fid)) {
			resp::resps::error_message(r.Err().msg).put(res);
			return HANDLER_ERROR;
		} else
			file = r.Ok();

		// Get file size
		int64_t fsize;
		if (auto r1 = file.details()) {
			plog::v(
				LOG_WARNING "fs", "Cannot get file details: %s", r1.Err().msg);
			resp::resps::error_message(r1.Err().msg).put(res);
			return HANDLER_ERROR;
		} else
			fsize = r1.Ok().size;

		{ // Send the file size to the client
			std::stringstream tmp;
			resp::resps::simple_string("OK").put(tmp);
			size_t _size = fsize; // the serializer needs to move the value
			resp::resps::integer(std::move(_size)).put(tmp);

			if (auto r = stream->write(
					tmp.str().c_str(),
					tmp.str().length())) // +OK\r\n:file length\r\n
				return helper::log_write_error(r.Err());
			DO_TIMEOUT;
		}

		{ // Handler client errors here
			std::unique_ptr<char[]> buf(new char[MAX_NET_MSG_LEN + 1]);
			if (auto r = stream->read(buf.get(), MAX_NET_MSG_LEN))
				return helper::log_read_error(r.Err());

			auto val = helper::get_value(buf.get());
			switch (resp::get_type(val)) {
				case resp::types::SIMPLE_STR:
					if (val.as_simple_string() != "OK") {
						plog::v(LOG_INFO "client", "Not ready");
						return HANDLER_NO_SEND_RES;
					}
					break;

				case resp::types::ERROR:
					plog::v(LOG_NOTICE "client",
							"%s",
							std::string(val.as_error_message()).c_str());
					return HANDLER_NO_SEND_RES;
					break;

				default:
					resp::resps::error_message("Invalid response").put(res);
					return HANDLER_ERROR;
					break;
			}
		}

		{ // Send the file data to the client
			std::stringstream tmp;
			resp::resps::simple_string("OK").put(tmp);
			// And for some manual serialization
			tmp << "$" << fsize << "\r\n";

			// Send the first part of the message
			if (auto r = stream->write(
					tmp.str().c_str(), tmp.str().length(), MSG_MORE))
				return helper::log_write_error(r.Err());

			// mmap() the file into memory
			int fd = open(file.path().c_str(), O_RDONLY);
			if (fd < 0) {
				plog::v(
					LOG_ERROR "fs", "Cannot open file: %s", Error(errno).msg);
				resp::resps::error_message(Error(errno).msg).put(tmp);
				return HANDLER_ERROR;
			}

			void* fptr =
				mmap(NULL, fsize, PROT_READ, MAP_PRIVATE | MAP_STACK, fd, 0);
			if (fptr == MAP_FAILED) {
				plog::v(
					LOG_ERROR "fs", "Cannot map file: %s", Error(errno).msg);
				resp::resps::error_message(Error(errno).msg);
				return HANDLER_ERROR;
			}

			if (auto r = stream->write((const char*)fptr, fsize, MSG_MORE))
				return helper::log_write_error(r.Err());

			munmap(fptr, fsize);
			close(fd);

			if (auto r = stream->write("\r\n", 2))
				return helper::log_write_error(r.Err());

			return HANDLER_SUCCESS;
		}
		*/
	}
} // namespace handlers