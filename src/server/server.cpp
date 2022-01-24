#include "nio/ip/v4/server.hpp"

#include <signal.h>

#include <iostream>
#include <string>

#include "handlers.hpp"
#include "log.hpp"
#include "misc.hpp"
#include "protocol.hpp"
#include "volume.hpp"

static void exit_handler(int sig) {
	plog::v(LOG_INFO "handler", "Exit signal detected. Exiting...");
	exit(sig);
}

volume::volume_type vol;

static protocol::cmd_table commands = {
	{protocol::commands::INVALID, handlers::invalid}, {1, handlers::get}};

int main() {
	// Register signal handlers for graceful exits
	signal(SIGINT, exit_handler);
	signal(SIGTERM, exit_handler);

	// Setup the volume
	if (auto r = volume::init("/tmp/testvol")) {
		plog::v(LOG_ERROR "volume", "Initialization: %s", r.Err().msg);
		exit(r.Err().no);
	} else
		vol = r.Ok();

	// Add a test file
	vol.store("test 1", "rick astley", strlen("rick astley"));

	// Setup network stuff
	nio::ip::v4::server srv(nio::ip::v4::addr("127.0.0.1", 8888));

	if (auto r = srv.Create()) {
		plog::v(LOG_ERROR "net", "Cannot create socket: %s", r.Err().msg);
		exit(r.Err().no);
	}

	int enable = 1;
	setsockopt(srv.raw(), SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));

	if (auto r = srv.Bind()) {
		plog::v(LOG_ERROR "net", "Cannot bind socket: %s", r.Err().msg);
		exit(r.Err().no);
	}

	if (auto r = srv.Listen()) {
		plog::v(LOG_ERROR "net", "Cannot listen on socket: %s", r.Err().msg);
		exit(r.Err().no);
	}

	// Main loop
	for (;;) {
		nio::ip::v4::stream stream;
		if (auto r = srv.Accept()) {
			plog::v(LOG_WARNING "net", "Cannot accept: %s", r.Err().msg);
			continue;
		} else
			stream = r.Ok();

		plog::v("", "--------------------");

		plog::v(LOG_INFO "net",
				"Connected: %s:%zu",
				stream.peer().ip().c_str(),
				stream.peer().port());

		// Read the message header
		protocol::types::header head;
		{
			char tmp[protocol::types::header::SIZE];
			if (auto r = stream.read(tmp, sizeof(tmp), MSG_WAITALL)) {
				plog::v(LOG_WARNING "net", "Cannot read: %s", r.Err().msg);
				continue;
			}

			if (protocol::get_type(tmp) != protocol::types::ids::HEADER) {
				plog::v(LOG_WARNING "client", "Bad request");
				continue;
			}

			const char* tmp1 = tmp;
			head.from(tmp1);
		}

		std::unique_ptr<char[]> req(new char[head.length]);
		if (auto r = stream.read(req.get(), head.length, MSG_WAITALL)) {
			plog::v(LOG_WARNING "net", "Cannot read: %s", r.Err().msg);
			continue;
		}

		std::stringstream res;

		auto result = protocol::parse(commands, head, req.get(), res, &stream);

		plog::v(LOG_INFO "parser", "Handler status: %d", result);

		// The handler is expected to return HANDLER_ERROR and set the desired
		// response in res if there is an error and the response isn't sent from
		// the handler
		if (result == HANDLER_ERROR) {
			if (auto r = stream.write(res.str().c_str(), res.str().length())) {
				plog::v(LOG_WARNING "net", "Cannot write: %s", r.Err().msg);
				continue;
			}
		}
	}
	return 0;
}