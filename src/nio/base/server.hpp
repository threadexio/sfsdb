#pragma once

#include "../error.hpp"
#include "socket.hpp"

namespace nio {
	namespace base {

		class server : public _sock {
			public:
			virtual error Bind()				 = 0;
			virtual error Listen(int _queue = 5) = 0;
		};

	} // namespace base
} // namespace nio