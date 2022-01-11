#include "nio/ip/v4/server.hpp"

#include <fcntl.h>
#include <signal.h>
#include <sys/mman.h>

#include <chrono>
#include <iostream>
#include <string>
#include <thread>

#include "log.hpp"
#include "resp.hpp"
#include "volume.hpp"

#define MAX_NET_MSG_LEN 255

#define HANDLER_ERROR		-1
#define HANDLER_NO_SEND_RES -2

#define MAX_NET_MSG_LEN 255
#define MSG_TIMEOUT_MS	20

#define DO_TIMEOUT                                                             \
	std::this_thread::sleep_for(std::chrono::milliseconds(MSG_TIMEOUT_MS))

static void exit_handler(int sig) {
	plog::v(LOG_INFO "handler", "Exit signal detected. Exiting...");
	exit(sig);
}

static volume::volume_type vol;

namespace handlers {

	static int invalid(std::stringstream&, std::stringstream&, void*) {
		plog::v(LOG_INFO "parser", "Invalid command");
		return 0;
	}

	static int get(std::stringstream& req,
				   std::stringstream& resfinal,
				   void*			  arg) {
		plog::v(LOG_INFO "parser", "GET command");

		auto* stream = (nio::base::stream<sockaddr>*)arg;

		uid::uid_type fid;

		try {
			fid = std::string(resp::value {req}.as_simple_string());
		} catch (const std::exception& e) {
			resp::resps::error_message("Invalid argument").put(resfinal);
			return HANDLER_ERROR;
		}

		// Find requested file
		size_t fsize;
		if (auto r = vol.get_id(fid)) {
			resp::resps::error_message(r.Err().msg).put(resfinal);
			return HANDLER_ERROR;
		} else {
			if (auto r1 = r.Ok().details()) {
				plog::v(
					LOG_WARNING "fs",
					std::string("Cannot get file details: ") + r1.Err().msg);
				resp::resps::error_message(r.Err().msg).put(resfinal);
				return HANDLER_ERROR;
			} else {
				// Send the file size to the client
				{
					std::stringstream res;
					fsize = r1.Ok().size;
					resp::resps::simple_string("OK").put(res);
					resp::resps::integer(r1.Ok().size).put(res);
					stream->write(res.str().c_str(), res.str().length());
					DO_TIMEOUT;
				}

				// Read the client's response and make sure it is ready
				try {
					std::unique_ptr<char[]> buf(new char[MAX_NET_MSG_LEN + 1]);
					stream->read(buf.get(), MAX_NET_MSG_LEN);
					std::stringstream res {buf.get()};

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
						resp::value {res}.get());
					if (! ok)
						return HANDLER_NO_SEND_RES;
				} catch (const std::exception& e) {
					plog::v(LOG_ERROR "net",
							std::string("Client: ") + e.what());
					return HANDLER_NO_SEND_RES;
				}

				// Send the file data to the client
				{
					std::stringstream res;
					resp::resps::simple_string("OK").put(res);
					// And for some manual serialization
					res << "$" << fsize << "\r\n";

					// mmap() the file into memory
					int fd = open(r.Ok().path().c_str(), O_RDONLY);
					if (fd < 0) {
						plog::v(LOG_ERROR "fs",
								std::string("Cannot open file: ") +
									Error(errno).msg);
						resp::resps::error_message(Error(errno).msg)
							.put(resfinal);
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

					res.write((const char*)fptr, fsize);

					munmap(fptr, fsize);
					close(fd);

					res << "\r\n";

					stream->write(res.str().c_str(), res.str().length());
					DO_TIMEOUT;
				}

				return 0;
			}
		}
	}
} // namespace handlers

static const resp::rcmd_t cmds[] = {{RESP_INV_HANDLER, handlers::invalid},
									{"GET", handlers::get}};

int main() {
	// Register signal handlers for graceful exits
	signal(SIGINT, exit_handler);
	signal(SIGTERM, exit_handler);

	resp::parser parser(cmds);

	// Setup the volume
	if (auto r = volume::init("/tmp/testvol")) {
		plog::v(LOG_ERROR "volume",
				"Cannot initialize volume: " + std::string(r.Err().msg));
		exit(r.Err().no);
	} else
		vol = r.Ok();

	// Add a test file
	vol.store("test file",
			  "this is some random test data",
			  strlen("this is some random test data"));

	// Setup network stuff
	nio::ip::v4::server srv(nio::ip::v4::addr("127.0.0.1", 8888));

	if (auto r = srv.Create()) {
		plog::v(LOG_ERROR "net",
				"Cannot create socket: " + std::string(r.Err().msg));
		exit(r.Err().no);
	}

	int enable = 1;
	setsockopt(srv.raw(), SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));

	if (auto r = srv.Bind()) {
		plog::v(LOG_ERROR "net", std::string(r.Err().msg));
		exit(r.Err().no);
	}

	if (auto r = srv.Listen()) {
		plog::v(LOG_ERROR "net",
				"Cannot listen on socket: " + std::string(r.Err().msg));
		exit(r.Err().no);
	}

	// Main loop
	for (;;) {
		nio::ip::v4::stream stream;
		if (auto r = srv.Accept()) {
			plog::v(LOG_WARNING "net",
					"Cannot accept: " + std::string(r.Err().msg));
			continue;
		} else
			stream = r.Ok();

		plog::v(LOG_INFO "net",
				"Connected: " + stream.peer().ip() + ":" +
					std::to_string(stream.peer().port()));

		// Read and parse a command
		std::unique_ptr<char[]> buf(new char[MAX_NET_MSG_LEN + 1]);
		if (auto r = stream.read(buf.get(), MAX_NET_MSG_LEN)) {
			plog::v(LOG_WARNING "net",
					std::string("Cannot read: ") + r.Err().msg);
			continue;
		}

		std::stringstream req {buf.get()};
		std::stringstream res;

		auto result = parser.parse(req, res, &stream);

		plog::v(LOG_INFO "parser", "Handler status: " + std::to_string(result));

		if (result != HANDLER_NO_SEND_RES) {
			if (auto r = stream.write(res.str().c_str(), res.str().length())) {
				plog::v(LOG_WARNING "net",
						std::string("Cannot write: ") + r.Err().msg);
				continue;
			}
		}
	}
	return 0;
}