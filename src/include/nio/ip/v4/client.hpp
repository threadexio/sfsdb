#pragma once

#include "../../base/client.hpp"
#include "../../error.hpp"
#include "addr.hpp"
#include "stream.hpp"

namespace nio {
	namespace ip {
		namespace v4 {
			/**
			 * @brief An IPv4 client.
			 */
			class client4 : public base::client<addr4, stream4> {
			public:
				/**
				 * @brief Create a new IPv4 client which will connect to
				 * _remote.
				 *
				 * @param _err Check this for any errors
				 * @param _remote
				 */
				client4(error& _err, addr4 _remote);

				/**
				 * @brief Connect to the endpoint specified.
				 *
				 * @param _err Check this for any errors
				 * @return stream4
				 */
				stream4 Connect(error& _err);
			};
		} // namespace v4
	}	  // namespace ip
} // namespace nio