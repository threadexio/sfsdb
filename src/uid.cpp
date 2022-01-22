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
		char _rdata[RDATA_LEN];
		rdev.read(_rdata, RDATA_LEN);

		uint64_t timestamp =
			std::chrono::duration_cast<std::chrono::microseconds>(
				std::chrono::system_clock::now().time_since_epoch())
				.count();

		uid_type ret;
		ret.resize(TOTAL_LEN);
		snprintf(ret.data(),
				 TOTAL_LEN + 1,
				 "%08x%c%04x%c%04x%c%04x%c%08x",
				 (uint32_t)(timestamp & 0x0000ffff),
				 SEPARATOR,
				 (uint16_t)(timestamp & 0x00ff0000),
				 SEPARATOR,
				 (uint16_t)(timestamp & 0xff000000),
				 SEPARATOR,
				 *(uint16_t *)(_rdata),
				 SEPARATOR,
				 *(uint32_t *)(_rdata + sizeof(uint16_t)));
		return ret;
	}
} // namespace uid
