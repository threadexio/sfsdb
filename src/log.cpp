#include "log.hpp"

#include <cstdio>
#include <ctime>

void plog::v(const std::string& _id, const std::string& _msg) {
	// Time information
	char   datebuf[64];
	time_t rawtime;
	time(&rawtime);
	tm* timezone = localtime(&rawtime);
	strftime(datebuf, sizeof(datebuf), "%b %d %X", timezone);

	fprintf(stderr,
			"%s %s: %s" LOG_COLOR_RESET "\n",
			datebuf,
			_id.c_str(),
			_msg.c_str());
}