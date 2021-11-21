#include "nio/ip/v4/client.hpp"

#include <cstdint>
#include <iostream>

#include "nio/buffer.hpp"
#include "nio/error.hpp"
#include "nio/ip/v4/addr.hpp"

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

	nio::buffer buf((void*)"1234567890123456", 16);
	stream.write(e, buf);

	std::cout << "Echoed: " << (char*)stream.read(e, 16).content() << "\n";

	return 0;
}