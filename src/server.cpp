#include "nio/ip/v4/server.hpp"

#include <cstdint>
#include <iostream>

#include "nio/buffer.hpp"
#include "nio/error.hpp"
#include "nio/ip/v4/addr.hpp"

int main() {
	auto e = nio::error();

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

		nio::buffer data = stream.read(e, 16);

		std::cout << "Recveived: " << (char*)data.content() << "\n";

		stream.write(e, data);

		stream.shutdown();
	}

	return 0;
}