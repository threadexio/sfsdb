#pragma once

#include "../buffer.hpp"
#include "socket.hpp"

namespace nio {
	namespace base {
		class stream : public _sock {};
	} // namespace base
} // namespace nio