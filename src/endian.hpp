#pragma once

#include <netinet/in.h>

#include <cstdint>

namespace endian {

	using long_type	 = uint32_t;
	using short_type = uint16_t;

	namespace host {
		inline long_type to_net(long_type net) {
			return htonl(net);
		}

		inline short_type to_net(short_type net) {
			return htons(net);
		}

		inline void to_net(char*& out, long_type net) {
			*(long_type*)out = to_net(net);
			out += sizeof(net);
		}

		inline void to_net(char*& out, short_type net) {
			*(short_type*)out = to_net(net);
			out += sizeof(net);
		}
	} // namespace host

	namespace net {
		inline long_type to_host(long_type net) {
			return ntohl(net);
		}

		inline short_type to_host(short_type net) {
			return ntohs(net);
		}

		inline void to_host(char*& out, long_type net) {
			*(long_type*)out = to_host(net);
			out += sizeof(net);
		}

		inline void to_host(char*& out, short_type net) {
			*(short_type*)out = to_host(net);
			out += sizeof(net);
		}
	} // namespace net

}; // namespace endian