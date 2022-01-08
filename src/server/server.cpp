#include "nio/ip/v4/server.hpp"

#include <signal.h>

#include <iostream>

#include "log.hpp"
#include "resp/resp.hpp"
#include "volume.hpp"

static void exit_handler(int sig) {
	plog::v(LOG_INFO "handler", "Exit signal detected. Exiting...");
	exit(sig);
}

static volume::volume_type vol;

// TODO: Improve the resp parser, like a lot.

//	Stuff to improve:
//	=================
//	1. Don't SEGV on a weird request
//	2. Make a better interface
//	3. Make a resizeable container (or reuse std::vector)

/**
 * @brief Get a file by ID
 *
 * @param data
 * @return int
 */
static int get(char* data, void* _stream) {
	auto* stream = (nio::ip::v4::stream*)_stream;

	resp::types::bulkstr fid = data;

	plog::v(LOG_INFO "parser", "Getting file: " + std::string(fid.value));

	nio::buffer buf(512);
	if (auto r = vol.get_id(fid.value)) {
		plog::v(LOG_WARNING "volume",
				"Cannot get file " + std::string(fid.value) + ": " +
					std::string(r.Err().msg));

		{
			char* head = buf;
			resp::types::error(r.Err().msg).serialize(head);
		}
		stream->write(buf);

		return r.Err().no;
	} else {
		auto fobj = r.Ok();
		auto fs	  = fobj.get();

		if (auto r1 = fobj.details()) {
			plog::v(LOG_ERROR "parser",
					"Cannot stat file: " + std::string(fid.value));

			{
				char* head = buf;
				resp::types::error(r.Err().msg).serialize(head);
			}
			stream->write(buf);
			return r.Err().no;
		} else {
			buf.resize(r1.Ok().size + 1);
			buf.at(r1.Ok().size) = 0;
			char* head			 = buf;
			fs.read(buf, r1.Ok().size);
			resp::types::bulkstr(buf).serialize(head);
			stream->write(buf);
			return 0;
		}
	}
}

static int invalid(char*, void*) {
	plog::v(LOG_WARNING "parser", "Invalid command.");
	return 1;
}

static const resp::rcmd_t cmds[] = {{"_INV", invalid}, {"GET", get}};
/*
_ERR is the callback for errors (not needed on the server)
_INV is the callback for invalid commands (not needed on the client)
*/

int main() {
	// Register signal handlers for graceful exits
	signal(SIGINT, exit_handler);
	signal(SIGTERM, exit_handler);

	// Setup resp parser
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
		nio::buffer data;
		if (auto r = stream.read(256)) {
			plog::v(LOG_WARNING "net",
					"Cannot read: " + std::string(r.Err().msg));
			continue;
		} else
			data = r.Ok();

		auto result = parser.parse(data, &stream);

		plog::v(LOG_INFO "parser", "parse result: " + std::to_string(result));
	}
	return 0;
}