#include "nio/ip/v4/client.hpp"

#include <iostream>

#include "log.hpp"
#include "resp.hpp"
#include "uid.hpp"

static int err(std::stringstream&) {
	return -1;
}

static int ok(std::stringstream&) {
	plog::v(LOG_INFO "parser", "Everything is ok");
	return 0;
}

static const resp::rcmd_t cmds[] = {{"_ERR", err}, {"OK", ok}};

int main() {
	nio::ip::v4::client cli(nio::ip::v4::addr("127.0.0.1", 8888));

	if (auto r = cli.Create()) {
		plog::v(LOG_ERROR "net",
				"Cannot create socket: " + std::string(r.Err().msg));
		exit(r.Err().no);
	}

	nio::ip::v4::stream stream;

	if (auto r = cli.Connect()) {
		plog::v(LOG_ERROR "net", "Cannot connect: " + std::string(r.Err().msg));
		exit(r.Err().no);
	} else
		stream = std::move(r.Ok());

	// Prepare our command
	nio::buffer buf(256);

	// Send our command
	if (auto r = stream.write(buf, strlen(buf))) {
		plog::v(LOG_WARNING "net", "Cannot send: " + std::string(r.Err().msg));
		exit(r.Err().no);
	}

	// Read and parse the response
	if (auto r = stream.read(256)) {
		plog::v(LOG_WARNING "net", "Cannot read: " + std::string(r.Err().msg));
		exit(r.Err().no);
	} else
		buf = r.Ok();

	auto result = -1;

	plog::v(LOG_INFO "net", "Parser: " + std::to_string(result));

	return 0;
}
