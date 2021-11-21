#pragma once

#include <netinet/in.h>

#include <string>

namespace nio {
	namespace base {

		/**
		 * @brief Base class for any address
		 *
		 * @tparam T Type of the underlying C struct
		 */
		template <class T>
		class addr {
			public:
			T saddr;

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

			virtual void _setup() = 0;
		};
	} // namespace base
} // namespace nio
