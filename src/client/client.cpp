#include "nio/ip/v4/client.hpp"

#include <iostream>

#include "log.hpp"
#include "resp/resp.hpp"
#include "uid.hpp"

static int err(char* data) {
	resp::types::error err(data);

	std::cout << "Error: " << err.value << "\n";

	return 1;
}

static int ok(char*) {
	std::cout << "everything is ok!\n";
	return 0;
}

static const resp::rcmd_t cmds[] = {{"_ERR", err}, {"OK", ok}};

int main() {
	resp::parser parser(cmds);

	auto cli = nio::ip::v4::client(nio::ip::v4::addr("127.0.0.1", 8888));

	nio::ip::v4::stream stream;

	if (auto r = cli.Connect()) {
		plog::v(LOG_ERROR "net", r.Err().msg);
		// stream.shutdown();
		exit(r.Err().no);
	} else
		stream = std::move(r.Ok());

	// Prepare our command
	nio::buffer buf(256);

	char* head = buf;

	resp::types::simstr("GET").serialize(head);
	resp::types::bulkstr(uid::generator().get().c_str()).serialize(head);

	// Send our command
	if (auto r = stream.write(buf, strlen(buf))) {
		plog::v(LOG_WARNING "net", r.Err().msg);
		// stream.shutdown();
		exit(r.Err().no);
	}

	// Read and parse the response
	if (auto r = stream.read(256)) {
		plog::v(LOG_WARNING "net", r.Err().msg);
		// stream.shutdown();
		exit(r.Err().no);
	} else
		buf = r.Ok();

	auto result = parser.parse(buf);

	std::cout << "response: " << result << "\n";

	if (result > 0) {
		stream.shutdown();
	}

	stream.shutdown();
	return 0;
}
