#include "nio/ip/v4/client.hpp"

#include <cstdint>
#include <iostream>

#include "nio/buffer.hpp"
#include "nio/error.hpp"
#include "nio/ip/v4/addr.hpp"
#include "resp/components.hpp"
#include "resp/resp.hpp"

static resp::status invalid(char* data) {
	std::cout << "invalid response\n";
	return resp::status::NCMD;
}

static resp::status err(char* data) {
	resp::components::error err(data);

	std::cout << "Error: " << err.value << "\n";

	return resp::status::OK;
}

static resp::status ok(char* data) {
	std::cout << "OK!\n";
	return resp::status::OK;
}

static const resp::rcmd_t cmds[] = {
	{"_ERR", err}, {"_INV", invalid}, {"OK", ok}};

int main() {
	auto e = nio::error();

	auto cli = nio::ip::v4::client4(e, nio::ip::v4::addr4("127.0.0.1", 8888));
	if (e) {
		std::cout << e.err << ": " << e.msg << "\n";
		return 1;
	}

	auto stream = cli.Connect(e);
	if (e) {
		std::cout << e.err << ": " << e.msg << "\n";
		return 1;
	}

	nio::buffer buf((void*)"+GET\r\n:1234\r\n", 14);
	stream.write(e, buf);

	buf.clear();
	buf = stream.read(e, 256);
	std::cout << (int)resp::parse(cmds, RESP_COUNT(cmds), buf) << "\n";

	return 0;
}