#pragma once

#include <chrono>
#include <cstdint>
#include <cstring>
#include <fstream>

/*
 * Format:
 * ------------------------------
 * timestamp_low		(32 bits)
 *  separator "-"
 * timestamp_mid		(16 bits)
 *  separator "-"
 * timestamp_high		(16 bits)
 *  separator "-"
 * random_data			(16 bits)
 *  separator "-"
 * random_data			(32 bits)
 * -------------------------------
 * total random data	(6 bytes)
 * total data bytes		(14 bytes)
 * total bytes			(18 bytes)
 *
 * total bytes of
 * hex representation	(32)
 */

#define UID_SEPARATOR '-'
#define UID_RDATA_LEN 6
#define UID_DATA_LEN  14
#define UID_TOTAL_LEN 32

#define UID_RAND_DEV "/dev/urandom"

namespace uid {

	using uid_type = std::string;

	class generator {
	public:
		/**
		 * @brief Create a new uid generator.
		 *
		 * @param dev File/Device to read random data from
		 */
		generator(const char *dev = UID_RAND_DEV);

		// std::fstream also deletes these, so we cant use them
		generator(const generator &other) noexcept = delete;
		generator(generator &&other) noexcept	   = delete;

		~generator();

		/**
		 * @brief Create a new uid.
		 *
		 * @return uid_type
		 */
		uid_type get();

	private:
		std::fstream rdev;
	};
} // namespace uid
