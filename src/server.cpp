#include "nio/ip/v4/server.hpp"

#include <asm-generic/socket.h>
#include <signal.h>
#include <sys/socket.h>

#include <cstdint>
#include <iostream>

#include "nio/buffer.hpp"
#include "nio/error.hpp"
#include "nio/ip/v4/addr.hpp"
#include "resp/resp.hpp"
#include "resp/types.hpp"

nio::error			 e;
nio::ip::v4::server4 srv(e, nio::ip::v4::addr4("127.0.0.1", 8888));

static void exit_handler(int sig) {
	srv.shutdown();
	std::cout << "\rExiting...\n";
	exit(1);
}

static int get(char* data) {
	resp::types::integer len(data);

	std::cout << "len = " << len.value << "\n";

	return 0;
}

static int invalid(char* data) {
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

	signal(SIGINT, exit_handler);
	signal(SIGTERM, exit_handler);

	resp::parser parser(cmds);

	std::cout << "bind(): " << srv.Bind().msg << "\n";

	std::cout << "listen(): " << srv.Listen().msg << "\n";

	for (;;) {
		auto stream = srv.Accept(e);
		if (e) {
			std::cout << "accept(): " << e.err << ":" << e.msg << "\n";
			srv.shutdown();
			return 1;
		}

		// Print peer info
		std::cout << "Connected: " << stream.peer().ip() << ":"
				  << stream.peer().port() << "\n";

		// Read and parse a command
		nio::buffer data   = stream.read(e, 256);
		auto		result = parser.parse(data);

		std::cout << "Parse result: " << result << "\n";

		// We need another pointer here because it is going to be changed by
		// serialize()
		char* head = data;

		// Do all of the data processing inside the callbacks and respond from
		// here
		if (result > 0) {
			resp::types::error("Some error message").serialize(head);
		} else {
			resp::types::simstr("OK").serialize(head);
		}

		stream.write(e, data);

		stream.shutdown();
	}

	return 0;
}