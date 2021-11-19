#include <iostream>

#include "nio/ip/v4/addr.hpp"

int main() {
	auto a = nio::ip::v4::addr4("1.2.3.4", 8080);
	std::cout << "a = " << a.ip() << ":" << a.port() << "\n";
}