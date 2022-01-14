#pragma once
#include "error.h"
#include "log.hpp"
#include "misc.hpp"
#include "resp.hpp"
#include "volume.hpp"

//==============//

#include "handlers/helper.hpp"

#define REGISTER_HANDLER(name)                                                 \
	inline int name(std::stringstream& req, std::stringstream& res, void* arg)

//==============//

#include "handlers/get.hpp"
