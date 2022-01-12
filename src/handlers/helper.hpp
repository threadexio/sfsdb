#pragma once

#define REGISTER_HANDLER(name)                                                 \
	inline int name(std::stringstream& req, std::stringstream& res, void* arg)
