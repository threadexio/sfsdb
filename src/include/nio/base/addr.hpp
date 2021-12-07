#pragma once

#include <netinet/in.h>

#include <string>

namespace nio {
	namespace base {

		/**
		 * @brief Base class for any address.
		 *
		 * @tparam T Type of the underlying C struct
		 */
		template <class T>
		class addr {
		public:
			/**
			 * @brief Get the address in a human readable way.
			 *
			 * @return std::string
			 */
			virtual std::string ip() const = 0;

			/**
			 * @brief Set the address from a string.
			 *
			 * @param _ip
			 */
			virtual void ip(const std::string& _ip) = 0;

			/**
			 * @brief Get the port number.
			 *
			 * @return in_port_t
			 */
			virtual in_port_t port() const = 0;

			/**
			 * @brief Set the port number.
			 *
			 * @param _port
			 */
			virtual void port(in_port_t _port) = 0;

			/**
			 * @brief Get the length of the underlying C struct. Only useful in
			 * syscalls.
			 *
			 * @return socklen_t&
			 */
			socklen_t& length() {
				return slen;
			};

			virtual operator sockaddr*() = 0;

		protected:
			T		  saddr;
			socklen_t slen;

			virtual void _setup() = 0;
		};
	} // namespace base
} // namespace nio
