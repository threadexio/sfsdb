#include "uid.hpp"

namespace uid {
	uid_type::uid_type(const char *_rdata) {
		uint64_t timestamp =
			std::chrono::duration_cast<std::chrono::milliseconds>(
				std::chrono::system_clock::now().time_since_epoch())
				.count();

		snprintf(_data,
				 UID_TOTAL_LEN + 1,
				 "%08x" UID_SEPARATOR "%04x" UID_SEPARATOR "%04x" UID_SEPARATOR
				 "%04x" UID_SEPARATOR "%08x",
				 (uint32_t)(timestamp & 0x0000ffff),
				 (uint16_t)(timestamp & 0x00ff0000),
				 (uint16_t)(timestamp & 0xff000000),
				 *(uint16_t *)(_rdata),
				 *(uint32_t *)(_rdata + sizeof(uint16_t)));
	}

	generator::generator(const char *dev) {
		rdev = std::fstream(dev, std::ios::in);
	}

	generator::~generator() {
		rdev.close();
	}
	uid_type generator::get() {
		char data[UID_RDATA_LEN];
		rdev.read(data, UID_RDATA_LEN);
		return uid_type(data);
	}
} // namespace uid
