#pragma once

#include "error.h"
#include "log.hpp"
#include "misc.hpp"
#include "protocol.hpp"
#include "volume.hpp"

//==============//

#define REGISTER_HANDLER(name)                                                 \
	inline int name(const protocol::header& head,                              \
					char*					req,                               \
					std::stringstream&		res,                               \
					void*					arg)

//==============//

#include "nio/base/stream.hpp"

namespace handlers {
	using stream_type = nio::base::stream<sockaddr>;
}