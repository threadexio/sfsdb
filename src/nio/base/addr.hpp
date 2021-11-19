#pragma once

#include <netinet/in.h>
#include <sys/socket.h>

#include <string>

namespace nio {
	namespace base {
		class addr {
			public:
			virtual std::string ip() const				   = 0;
			virtual void		ip(const std::string& _ip) = 0;

			virtual in_port_t port() const			= 0;
			virtual void	  port(in_port_t _port) = 0;

			socklen_t& length() {
				return slen;
			};

			virtual operator sockaddr*() = 0;

			protected:
			socklen_t slen;
		};
	} // namespace base
} // namespace nio
