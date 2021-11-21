#pragma once

#include <netinet/in.h>

#include <string>

#include "../../base/addr.hpp"

namespace nio {
	namespace ip {
		namespace v4 {
			class addr4 : public base::addr<sockaddr_in> {
				public:
				addr4();

				addr4(const std::string& _ip, in_port_t _port);

				std::string ip() const;
				void		ip(const std::string& _ip);

				in_port_t port() const;
				void	  port(in_port_t _port);

				operator sockaddr*();

				private:
				void _setup();
			};
		} // namespace v4
	}	  // namespace ip
} // namespace nio