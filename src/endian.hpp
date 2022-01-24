#pragma once

#include <netinet/in.h>

#include <cstdint>
#include <sstream>

namespace endian {

	using long_type	 = uint32_t;
	using short_type = uint16_t;

	namespace host {
		inline long_type to_netl(long_type net) {
			return htonl(net);
		}

		inline short_type to_nets(short_type net) {
			return htons(net);
		}

		// Input

		// Convert to network from buffer
		inline long_type to_netl(const char*& in) {
			auto ret = to_netl(*(long_type*)in);
			in += sizeof(ret);
			return ret;
		}

		// Convert to network from buffer
		inline short_type to_nets(const char*& in) {
			auto ret = to_nets(*(short_type*)in);
			in += sizeof(ret);
			return ret;
		}

		// Convert to network from stream
		inline long_type to_netl(std::stringstream& in) {
			long_type tmp;
			in.read((char*)&tmp, sizeof(tmp));
			return to_netl(tmp);
		}

		// Convert to network from stream
		inline short_type to_nets(std::stringstream& in) {
			short_type tmp;
			in.read((char*)&tmp, sizeof(tmp));
			return to_nets(tmp);
		}

		// Output

		// Convert to network and write to buffer
		inline void to_netl(char*& out, long_type net) {
			*(long_type*)out = to_netl(net);
			out += sizeof(net);
		}

		// Convert to network and write to buffer
		inline void to_nets(char*& out, short_type net) {
			*(short_type*)out = to_nets(net);
			out += sizeof(net);
		}

		// Convert to network and write to stream
		inline void to_netl(std::stringstream& out, long_type net) {
			auto tmp = to_netl(net);
			out.write((const char*)&tmp, sizeof(tmp));
		}

		// Convert to network and write to stream
		inline void to_nets(std::stringstream& out, short_type net) {
			auto tmp = to_nets(net);
			out.write((const char*)&tmp, sizeof(tmp));
		}
	} // namespace host

	namespace net {
		inline long_type to_hostl(long_type net) {
			return ntohl(net);
		}

		inline short_type to_hosts(short_type net) {
			return ntohs(net);
		}

		// Input

		inline long_type to_hostl(const char*& in) {
			auto ret = to_hostl(*(long_type*)in);
			in += sizeof(ret);
			return ret;
		}

		inline short_type to_hosts(const char*& in) {
			auto ret = to_hosts(*(short_type*)in);
			in += sizeof(ret);
			return ret;
		}

		inline long_type to_hostl(std::stringstream& in) {
			long_type tmp;
			in.read((char*)&tmp, sizeof(tmp));
			return to_hostl(tmp);
		}

		inline short_type to_hosts(std::stringstream& in) {
			short_type tmp;
			in.read((char*)&tmp, sizeof(tmp));
			return to_hosts(tmp);
		}

		// Output

		inline void to_hostl(char*& out, long_type net) {
			*(long_type*)out = to_hostl(net);
			out += sizeof(net);
		}

		inline void to_hosts(char*& out, short_type net) {
			*(short_type*)out = to_hosts(net);
			out += sizeof(net);
		}

		inline void to_hostl(std::stringstream& out, long_type net) {
			auto tmp = to_hostl(net);
			out.write((const char*)&tmp, sizeof(tmp));
		}

		inline void to_hosts(std::stringstream& out, short_type net) {
			auto tmp = to_hosts(net);
			out.write((const char*)&tmp, sizeof(tmp));
		}
	} // namespace net

}; // namespace endian