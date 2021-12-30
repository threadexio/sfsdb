#pragma once

#include <chrono>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <string>

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

#define UID_SEPARATOR "-"
#define UID_RDATA_LEN 6
#define UID_DATA_LEN  14
#define UID_TOTAL_LEN 32

#define UID_RAND_DEV "/dev/urandom"

namespace uid {

	/**
	 * @brief Custom type to hold uids. std::string uses malloc(), not needed
	 * for such small strings which can be on the stack.
	 */
	class uid_type {
	public:
		/**
		 * @brief Create a new uid from random data.
		 *
		 * @param _rdata Pointer to the random data
		 */
		uid_type(const char *_rdata) {
			uint64_t timestamp =
				std::chrono::duration_cast<std::chrono::milliseconds>(
					std::chrono::system_clock::now().time_since_epoch())
					.count();

			snprintf(_data,
					 UID_TOTAL_LEN + 1,
					 "%08x" UID_SEPARATOR "%04x" UID_SEPARATOR
					 "%04x" UID_SEPARATOR "%04x" UID_SEPARATOR "%08x",
					 (uint32_t)(timestamp & 0x0000ffff),
					 (uint16_t)(timestamp & 0x00ff0000),
					 (uint16_t)(timestamp & 0xff000000),
					 *(uint16_t *)(_rdata),
					 *(uint32_t *)(_rdata + sizeof(uint16_t)));
		}

		inline operator const char *() const {
			return _data;
		}

	private:
		char _data[UID_TOTAL_LEN + 1];
	};

	class generator {
	public:
		/**
		 * @brief Create a new uid generator.
		 *
		 * @param dev File/Device to read random data from
		 */
		generator(const char *dev = UID_RAND_DEV) {
			rdev = std::fstream(dev, std::ios::in);
		}

		// std::fstream also deletes these, so we cant use them
		generator(const generator &other) noexcept = delete;
		generator(generator &&other) noexcept	   = delete;

		~generator() {
			rdev.close();
		}

		/**
		 * @brief Create a new uid.
		 *
		 * @return uid_type
		 */
		uid_type get() {
			char data[UID_RDATA_LEN];
			rdev.read(data, UID_RDATA_LEN);
			return uid_type(data);
		}

	private:
		std::fstream rdev;
	};
} // namespace uid
