#include "nio/ip/v4/client.hpp"

#include <iostream>

#include "resp/resp.hpp"
#include "resp/types.hpp"
#include "uid.hpp"

static int err(char* data) {
	resp::types::error err(data);

	std::cout << "Error: " << err.value << "\n";

	return 1;
}

static int ok(char* data) {
	std::cout << "everything is ok!\n";
	return 0;
}

static const resp::rcmd_t cmds[] = {{"_ERR", err}, {"OK", ok}};

int main() {
	uid::generator uidgen;

	auto e = nio::error();

	resp::parser parser(cmds);

	auto cli = nio::ip::v4::client(e, nio::ip::v4::addr("127.0.0.1", 8888));
	if (e) {
		std::cout << e.err << ": " << e.msg << "\n";
		return 1;
	}

	auto stream = cli.Connect(e);
	if (e) {
		std::cout << e.err << ": " << e.msg << "\n";
		return 1;
	}

	// Prepare our command
	nio::buffer buf(256);

	char* head = buf;

	resp::types::simstr("GET").serialize(head);
	resp::types::bulkstr(uidgen.get()).serialize(head);

	// Send our command
	stream.write(e, buf, strlen(buf));

	// Read and parse the response
	buf			= stream.read(e, 256);
	auto result = parser.parse(buf);

	std::cout << "response: " << result << "\n";

	if (result > 0) {
		stream.shutdown();
	}

	return 0;
}
