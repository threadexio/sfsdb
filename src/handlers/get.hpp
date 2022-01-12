#pragma once
#include "handlers.hpp"
#include "helper.hpp"

//=========//

#include <fcntl.h>
#include <signal.h>
#include <sys/mman.h>

#include "log.hpp"
#include "nio/base/stream.hpp"

extern volume::volume_type vol;

namespace handlers {
	REGISTER_HANDLER(get) {
		/**
		 * Command format:
		 *
		 * GET [file id]
		 */

		plog::v(LOG_INFO "parser", "GET command");

		auto*		  stream = (nio::base::stream<sockaddr>*)arg;
		uid::uid_type fid;

		// Parse command arguments
		try {
			fid = std::string(resp::value {req}.as_simple_string());
		} catch (const std::exception& e) {
			resp::resps::error_message("Invalid argument").put(res);
			return HANDLER_ERROR;
		}

		// Find requested file
		if (auto r = vol.get_id(fid)) {
			resp::resps::error_message(r.Err().msg).put(res);
			return HANDLER_ERROR;
		} else {
			if (auto r1 = r.Ok().details()) {
				plog::v(
					LOG_WARNING "fs",
					std::string("Cannot get file details: ") + r1.Err().msg);
				resp::resps::error_message(r.Err().msg).put(res);
				return HANDLER_ERROR;
			} else {
				int64_t fsize;
				{ // Send the file size to the client
					std::stringstream tmp;
					fsize = r1.Ok().size;
					resp::resps::simple_string("OK").put(tmp);
					resp::resps::integer(r1.Ok().size).put(tmp);
					stream->write(tmp.str().c_str(), tmp.str().length());
					DO_TIMEOUT;
				}

				try { // Read the client's response and make sure it is ready
					std::unique_ptr<char[]> buf(new char[MAX_NET_MSG_LEN + 1]);
					stream->read(buf.get(), MAX_NET_MSG_LEN);
					std::stringstream tmp {buf.get()};

					bool ok = false;
					std::visit(
						rediscpp::resp::detail::overloaded {
							[&](resp::respds::simple_string const& val) {
								if (val.get() == "OK")
									ok = true;
							},
							[&](resp::respds::error_message const& val) {
								plog::v(LOG_ERROR "client",
										std::string(val.get()));
							},
							[&](auto const&) {
								throw std::invalid_argument("Invalid response");
							}},
						resp::value {tmp}.get());
					if (! ok)
						return HANDLER_NO_SEND_RES;
				} catch (const std::exception& e) {
					plog::v(LOG_ERROR "net",
							std::string("Client: ") + e.what());
					return HANDLER_NO_SEND_RES;
				}

				{ // Send the file data to the client
					std::stringstream tmp;
					resp::resps::simple_string("OK").put(tmp);
					// And for some manual serialization
					tmp << "$" << fsize << "\r\n";

					// Send the first part of the message
					stream->write(
						tmp.str().c_str(), tmp.str().length(), MSG_MORE);

					// mmap() the file into memory
					int fd = open(r.Ok().path().c_str(), O_RDONLY);
					if (fd < 0) {
						plog::v(LOG_ERROR "fs",
								std::string("Cannot open file: ") +
									Error(errno).msg);
						resp::resps::error_message(Error(errno).msg).put(tmp);
						return HANDLER_ERROR;
					}

					void* fptr = mmap(
						NULL, fsize, PROT_READ, MAP_PRIVATE | MAP_STACK, fd, 0);
					if (fptr == MAP_FAILED) {
						plog::v(LOG_ERROR "fs",
								std::string("Cannot map file: ") +
									Error(errno).msg);
						resp::resps::error_message(Error(errno).msg);
						return HANDLER_ERROR;
					}

					stream->write((const char*)fptr, fsize, MSG_MORE);

					munmap(fptr, fsize);
					close(fd);

					stream->write("\r\n", 2);
				}

				return HANDLER_NO_SEND_RES;
			}
		}
	}
} // namespace handlers