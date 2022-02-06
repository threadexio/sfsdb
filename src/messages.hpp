#pragma once

#include "protocol.hpp"

namespace protocol {
	namespace messages {
		struct error {
			protocol::types::header head;
			protocol::types::error	err;

			error(const char* msg)
				: head(protocol::status::ERROR, 0), err(msg) {
				head.length = protocol::types::error::HEADER_SIZE + err.length;
			}

			void to(std::stringstream& out) {
				head.to(out);
				err.to(out);
			}

			bool from(const char*& in) {
				return head.from(in) && err.from(in);
			}
		};
	} // namespace messages
} // namespace protocol