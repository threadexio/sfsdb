#include "nio/ip/v4/server.hpp"

#include <signal.h>

#include <iostream>

#include "log.hpp"
#include "volume.hpp"

static void exit_handler(int sig) {
	plog::v(LOG_INFO "handler", "Exit signal detected. Exiting...");
	exit(sig);
}

static volume::volume_type vol;

int main() {
	// Register signal handlers for graceful exits
	signal(SIGINT, exit_handler);
	signal(SIGTERM, exit_handler);

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
		nio::buffer data;
		if (auto r = stream.read(256)) {
			plog::v(LOG_WARNING "net",
					"Cannot read: " + std::string(r.Err().msg));
			continue;
		} else
			data = r.Ok();

		// auto result = parser.parse(data, &stream);

		plog::v(LOG_INFO "parser", "parse result: " + std::to_string(-1));
	}
	return 0;
}