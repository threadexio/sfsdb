#include "ip/v4/addr.hpp"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>

nio::ip::v4::addr4::addr4() {
}

nio::ip::v4::addr4::addr4(const std::string& _ip, in_port_t _port) {
	slen = sizeof(sockaddr_in);
	memset(&saddr, 0, slen);
	saddr.sin_family = AF_INET;

	ip(_ip);
	port(_port);
}

std::string nio::ip::v4::addr4::ip() const {
	return inet_ntoa(saddr.sin_addr);
}

void nio::ip::v4::addr4::ip(const std::string& _ip) {
	saddr.sin_addr.s_addr = inet_addr(_ip.c_str());
}

in_port_t nio::ip::v4::addr4::port() const {
	return ntohs(saddr.sin_port);
}

void nio::ip::v4::addr4::port(in_port_t _port) {
	saddr.sin_port = htons(_port);
}

nio::ip::v4::addr4::operator sockaddr*() {
	return (sockaddr*)&saddr;
}