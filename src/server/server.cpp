#include "nio/ip/v4/server.hpp"

#include <signal.h>

#include <iostream>
#include <string>

#include "handlers.hpp"
#include "log.hpp"
#include "misc.hpp"
#include "volume.hpp"

static void exit_handler(int sig) {
	plog::v(LOG_INFO "handler", "Exit signal detected. Exiting...");
	exit(sig);
}

volume::volume_type vol;

namespace handlers {

	static int invalid(std::stringstream&, std::stringstream&, void*) {
		plog::v(LOG_INFO "parser", "Invalid command");
		return 0;
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
	vol.store("test 1", "rick astley", strlen("rick astley"));

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