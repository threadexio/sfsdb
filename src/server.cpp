#include "nio/ip/v4/server.hpp"

#include <cstdint>
#include <iostream>

#include "nio/buffer.hpp"
#include "nio/error.hpp"
#include "nio/ip/v4/addr.hpp"
#include "resp/components.hpp"
#include "resp/resp.hpp"

static resp::status get(char* data) {
	resp::components::integer len(data);

	std::cout << "len = " << len.value << "\n";

	return resp::status::OK;
}

static resp::status invalid(char* data) {
	std::cout << "invalid command\n";
	return resp::status::ECMD;
}

static resp::status error(char* data) {
	std::cout << "error\n";
	return resp::status::ERR;
}

static const resp::rcmd_t cmds[] = {
	{"_ERR", error}, {"_INV", invalid}, {"GET", get}};

int main() {
	auto		 e = nio::error();
	resp::parser parser(cmds);

	auto srv = nio::ip::v4::server4(e, nio::ip::v4::addr4("127.0.0.1", 8888));

	std::cout << "bind(): " << srv.Bind().msg << "\n";

	std::cout << "listen(): " << srv.Listen().msg << "\n";

	for (;;) {
		auto stream = srv.Accept(e);
		if (e) {
			std::cout << "accept(): " << e.err << ":" << e.msg << "\n";
			srv.shutdown();
			return 1;
		}
		std::cout << "Connected: " << stream.peer().ip() << ":"
				  << stream.peer().port() << "\n";

		nio::buffer data = stream.read(e, 256, 0);

		parser.parse(data);

		data.clear();
		data.write("+OK\r\n");
		std::cout << data.length() << "\n";
		stream.write(e, data);

		stream.shutdown();
	}

	return 0;
}