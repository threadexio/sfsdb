#pragma once

#include "../../base/server.hpp"
#include "../../error.hpp"
#include "addr.hpp"
#include "stream.hpp"

namespace nio {
	namespace ip {
		namespace v4 {
			/**
			 * @brief An IPv4 server.
			 */
			class server : public base::server<addr, stream> {
			public:
				/**
				 * @brief Create a new IPv4 server that will listen on _server.
				 *
				 * @param _err Check this for any errors
				 * @param _server
				 */
				server(error& _err, const addr& _server);

				/**
				 * @brief Bind to the specified address and get ready to listen.
				 * Should be called after the constructor.
				 *
				 * @return error - Check this for any errors
				 */
				error Bind();

				/**
				 * @brief Listen on the specified address and get ready to
				 * accept incoming connections.
				 *
				 * @param _queue
				 * @return error - Check this for any errors
				 */
				error Listen(int _queue = 5);

				/**
				 * @brief Accept an incoming connection.
				 *
				 * @param _err Check this for any errors
				 * @return stream - The new connection.
				 */
				stream Accept(error& _err);
			};
		} // namespace v4
	}	  // namespace ip
} // namespace nio