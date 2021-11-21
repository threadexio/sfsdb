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

				/**
				 * @brief Create a new addr4 object the represents the endpoint
				 * _ip:_port
				 *
				 * @param _ip
				 * @param _port
				 */
				addr4(const std::string& _ip, in_port_t _port);

				/**
				 * @brief Get the IP address.
				 *
				 * @return std::string
				 */
				std::string ip() const;

				/**
				 * @brief Set the IP address.
				 *
				 * @param _ip
				 */
				void ip(const std::string& _ip);

				/**
				 * @brief Get the port number.
				 *
				 * @return in_port_t
				 */
				in_port_t port() const;

				/**
				 * @brief Set the port number.
				 *
				 * @param _port
				 */
				void port(in_port_t _port);

				operator sockaddr*();

				private:
				void _setup();
			};
		} // namespace v4
	}	  // namespace ip
} // namespace nio