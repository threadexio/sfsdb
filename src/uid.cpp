#include "uid.hpp"

#include <chrono>
#include <cstring>

namespace uid {
	generator::generator(const char *dev) {
		rdev = std::fstream(dev, std::ios::in);
	}

	generator::~generator() {
		rdev.close();
	}
	uid_type generator::get() {
		char _rdata[UID_RDATA_LEN];
		rdev.read(_rdata, UID_RDATA_LEN);

		uint64_t timestamp =
			std::chrono::duration_cast<std::chrono::microseconds>(
				std::chrono::system_clock::now().time_since_epoch())
				.count();

		uid_type ret;
		ret.resize(UID_TOTAL_LEN);
		snprintf(ret.data(),
				 UID_TOTAL_LEN + 1,
				 "%08x%c%04x%c%04x%c%04x%c%08x",
				 (uint32_t)(timestamp & 0x0000ffff),
				 UID_SEPARATOR,
				 (uint16_t)(timestamp & 0x00ff0000),
				 UID_SEPARATOR,
				 (uint16_t)(timestamp & 0xff000000),
				 UID_SEPARATOR,
				 *(uint16_t *)(_rdata),
				 UID_SEPARATOR,
				 *(uint32_t *)(_rdata + sizeof(uint16_t)));
		return ret;
	}
} // namespace uid
