#include "protocol.hpp"

#include <cstring>

#include "endian.hpp"

namespace protocol {

	void header::to(char* out) const {
		endian::host::to_net(out, command);
		endian::host::to_net(out, length);
	}

	void header::to(std::stringstream& out) const {
		char  tmp[HEADER_SIZE];
		char* t = tmp;

		endian::host::to_net(t, command);
		endian::host::to_net(t, length);

		out.write(tmp, sizeof(tmp));
	}

	header header::from(const char* _data) {
		header ret;

		ret.command = endian::net::to_host(*(uint16_t*)_data);
		_data += sizeof(ret.command);

		ret.length = endian::net::to_host(*(uint32_t*)_data);
		_data += sizeof(ret.length);

		return ret;
	}

	int parse(const cmd_table&	 commands,
			  const header&		 head,
			  char*				 req,
			  std::stringstream& res,
			  void*				 arg) {
		cb_t invalid = nullptr;

		auto r = head;

		for (auto& c : commands) {
			if (c.no == INVALID_HANDLER)
				invalid = c.handler;

			if (c.no == r.command)
				if (c.handler != nullptr)
					return c.handler(head, req, res, arg);
		}

		if (invalid != nullptr)
			return invalid(head, req, res, arg);

		return -1;
	}

	namespace types {
		error::error() {
		}

		error::error(const char* _msg) {
			head.command = ERROR_HANDLER;
			head.length	 = strnlen(_msg, sizeof(msg) - 1);
			strncpy(msg, _msg, head.length);
		}

		void error::to(std::stringstream& out) const {
			head.to(out);

			out << msg;
		}

		error error::from(char*& _data) {
			error ret;

			ret.head = header::from(_data);
			strncpy(ret.msg,
					_data,
					(ret.head.length < sizeof(msg) - 1) ? sizeof(msg) - 1
														: ret.head.length);

			return ret;
		}

	} // namespace types

} // namespace protocol