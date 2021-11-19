#pragma once

#include "../../base/server.hpp"
#include "../../error.hpp"
#include "addr.hpp"
#include "stream.hpp"

namespace nio {
	namespace ip {
		namespace v4 {
			class server4 : public base::server {
				public:
				server4(error& _err, const addr4& _server);

				error Bind();
				error Listen(int _queue = 5);

				stream4 Accept(error& _err);

				private:
				addr4 server;
			};
		} // namespace v4
	}	  // namespace ip
} // namespace nio