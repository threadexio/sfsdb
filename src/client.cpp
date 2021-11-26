#include "nio/ip/v4/client.hpp"

#include <cstdint>
#include <iostream>

#include "nio/buffer.hpp"
#include "nio/error.hpp"
#include "nio/ip/v4/addr.hpp"
#include "resp/resp.hpp"

static resp::status invalid(char* data) {
	std::cout << "invalid command\n";
	return resp::status::NCMD;
}

static resp::status get(char* data) {
	std::cout << "get command\n";

	return resp::status::OK;
}

int main() {
	/*
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

	nio::buffer buf((void*)"1234567890123456", 16);
	stream.write(e, buf);

	std::cout << "Echoed: " << (char*)stream.read(e, 16).content() << "\n";

	return 0;
	*/

	const char* test = "+GET\r\n:-1234\r\n$17\r\nthis\n\r is my "
					   "data\r\n:123456789\r\n+simple string\r\n";

	resp::callbacks cbs;
	cbs.INV = invalid;
	cbs.GET = get;

	std::cout << "resp::parse = " << (int)resp::parse(cbs, (char*)test) << "\n";
}