#include "log.hpp"

#include <cstdarg>
#include <cstdio>
#include <ctime>

void plog::v(const char* _id, const char* fmt, ...) {
	// Time information
	char   datebuf[64];
	time_t rawtime;
	time(&rawtime);
	tm* timezone = localtime(&rawtime);
	strftime(datebuf, sizeof(datebuf), "%b %d %X", timezone);

	std::string format = datebuf;
	format += " ";
	format += _id;
	format += ": " + std::string(fmt) + LOG_COLOR_RESET "\n";

	va_list vl;
	va_start(vl, fmt);
	vfprintf(stderr, format.c_str(), vl);
	va_end(vl);
}