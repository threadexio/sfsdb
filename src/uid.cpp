#include "uid.hpp"

#include <chrono>
#include <cstdint>
#include <cstdio>
#include <string>

/*
 * Format:
 * 		timestamp (64 bits)
 * 		separator ("-")
 * 		random data (16 bits)
 * 		separator ("-")
 * 		random data (16 bits)
 * 		separator ("-")
 * 		random data (32 bits)
 */

#define UID_DATA 16

// Length of the uid string
#define UID_SIZE 36

namespace uid {
	generator::generator(const char *dev) {
		rdev = std::fopen(dev, "r");
	}

	generator::~generator() {
		fclose(rdev);
	}

	std::string generator::generate() {
		char rdata[UID_DATA];
		std::fread(rdata, UID_DATA, 1, rdev);

		uint64_t timestamp =
			std::chrono::duration_cast<std::chrono::milliseconds>(
				std::chrono::system_clock::now().time_since_epoch())
				.count();

		char retbuf[36];
		snprintf(retbuf,
				 UID_SIZE,
				 "%016lx-%04x-%04x-%08x",
				 timestamp,
				 *(uint16_t *)(rdata + 8),
				 *(uint16_t *)(rdata + 8 + 2),
				 *(uint32_t *)(rdata + 8 + 2 + 2));

		return std::string(retbuf, UID_SIZE);
	}
} // namespace uid
