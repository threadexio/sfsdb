#include "nio/ip/v4/server.hpp"

#include <signal.h>

#include <iostream>

#include "log.hpp"
#include "resp/resp.hpp"

nio::ip::v4::server srv(nio::ip::v4::addr("127.0.0.1", 8888));

static void exit_handler(int) {
	srv.shutdown();
	std::cout << "\rExiting...\n";
	exit(1);
}

static int get(char* data) {
	// resp::types::integer len(data);
	resp::types::bulkstr uid = data;

	std::cout << "Uid: " << uid.value << "\n";
	return 0;
}

static int invalid(char*) {
	std::cout << "invalid command\n";
	return 1;
}

static const resp::rcmd_t cmds[] = {{"_INV", invalid}, {"GET", get}};
/*
_ERR is the callback for errors (not needed on the server)
_INV is the callback for invalid commands (not needed on the client)
*/

int main() {
	int enable = 1;
	setsockopt(srv.raw(), SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));

	if (auto r = srv.Bind()) {
		plog::v(LOG_ERROR "net", r.Err().msg);
		srv.shutdown();
		exit(r.Err().no);
	}

	if (auto r = srv.Listen()) {
		plog::v(LOG_ERROR "net", r.Err().msg);
		srv.shutdown();
		exit(r.Err().no);
	}

	signal(SIGINT, exit_handler);
	signal(SIGTERM, exit_handler);

	resp::parser parser(cmds);

	for (;;) {
		nio::ip::v4::stream stream;
		if (auto r = srv.Accept()) {
			plog::v(LOG_ERROR "net", r.Err().msg);
			continue;
		} else
			stream = std::move(r.Ok());

		plog::v(LOG_INFO "net",
				std::string("Connected: " + stream.peer().ip() + ":" +
							std::to_string(stream.peer().port()))
					.c_str());

		// Read and parse a command
		nio::buffer data;
		if (auto r = stream.read(256)) {
			plog::v(LOG_WARNING "net", r.Err().msg);
			stream.shutdown();
			continue;
		} else
			data = r.Ok();

		auto result = parser.parse(data);

		// We need another pointer here because it is going to be changed by
		// serialize()
		char* head = data;

		// Do all of the data processing inside the callbacks and respond from
		// here
		if (result % 2) {
			resp::types::error("Some error message").serialize(head);
		} else {
			resp::types::simstr("OK").serialize(head);
		}

		if (auto r = stream.write(data, strlen(data))) {
			plog::v(LOG_WARNING "net", r.Err().msg);
			stream.shutdown();
			continue;
		}

		stream.shutdown();
	}

	return 0;
}
