#pragma once

#include "../../base/client.hpp"
#include "../../error.hpp"
#include "addr.hpp"
#include "stream.hpp"

namespace nio {
	namespace ip {
		namespace v4 {
			class client4 : public base::client<addr4, stream4> {
				public:
				client4(error& _err, addr4 _remote);

				stream4 Connect(error& _err);
			};
		} // namespace v4
	}	  // namespace ip
} // namespace nio