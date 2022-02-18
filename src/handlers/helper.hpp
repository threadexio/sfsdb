#pragma once

#include "error.h"
#include "log.hpp"
#include "messages.hpp"
#include "misc.hpp"
#include "nio/error.hpp"
#include "nio/stream.hpp"
#include "volume.hpp"

//==============//

#define REGISTER_HANDLER(name)                                                 \
	inline int name(const protocol::types::header& head,                       \
					const char*					   req,                        \
					std::stringstream&			   res,                        \
					void*						   arg)

//==============//

namespace handlers {
	using stream_type = nio::stream;
}