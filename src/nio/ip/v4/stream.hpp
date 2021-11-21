#pragma once

#include "../../base/stream.hpp"
#include "addr.hpp"

namespace nio {
	namespace ip {
		namespace v4 {
			class stream4 : public base::stream<addr4> {
				public:
				stream4(int _sock, const addr4& _p);
			};
		} // namespace v4
	}	  // namespace ip
} // namespace nio