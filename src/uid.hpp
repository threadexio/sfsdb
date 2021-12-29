#pragma once

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

namespace uid {
	class generator {
	public:
		generator(const char *dev = "/dev/urandom");

		~generator();

		std::string generate();

	private:
		FILE *rdev;
	};
} // namespace uid
