#pragma once

#include <sys/stat.h>

#include <filesystem>
#include <fstream>
#include <string>

#include "common.hpp"

#define STOR_DATA_DIR "data/"

namespace storage {

	constexpr auto DATA_DIR = "data/";

	struct meta {
		// Last access time (microseconds)
		uint64_t atime;
		// Last modification time (microseconds)
		uint64_t mtime;
		// Size of file
		uint64_t size;

		// Only for use with Result
		meta() {
		}

		meta(const struct stat& _stat);
	};

	/**
	 * @brief Create directory structure @ cwd.
	 *
	 */
	inline Result<void*, Error> init() {
		Result<void*, Error> ret;
		try {
			std::filesystem::create_directory(STOR_DATA_DIR);
			return std::move(ret.Ok(nullptr));
		} catch (const std::filesystem::filesystem_error& e) {
			return std::move(ret.Err(e.code().value()));
		}
	}

	struct data_type {
	private:
		std::string dname;
		std::string fpath;

	public:
		// Only for Result
		data_type() {};

		/**
		 * @brief Open or create a new data object at _dname.
		 *
		 * @param _dname
		 */
		data_type(const std::string& _dname);

		/**
		 * @brief Save data in the object. Will overwrite previous data!
		 *
		 * @param buf Buffer containing the data
		 * @param len Length of the data
		 */
		Result<void*, Error> save(const char* const buf, size_t len);

		/**
		 * @brief Remove the current data.
		 *
		 */
		inline Result<void*, Error> remove() {
			Result<void*, Error> ret;
			try {
				std::filesystem::remove(STOR_DATA_DIR + dname);
				return std::move(ret.Ok(nullptr));
			} catch (const std::filesystem::filesystem_error& e) {
				return std::move(ret.Err(e.code().value()));
			}
		}

		inline Result<meta, Error> details() {
			Result<meta, Error> ret;

			struct stat stbuf;
			if (stat((STOR_DATA_DIR + dname).c_str(), &stbuf) < 0)
				return std::move(ret.Err(errno));

			return std::move(ret.Ok(stbuf));
		}

		/**
		 * @brief Get a the file data stream.
		 *
		 * @return std::fstream
		 */
		inline std::fstream get() {
			return std::fstream(STOR_DATA_DIR + dname,
								std::ios::in | std::ios::binary);
		}

		/**
		 * @brief Get file path.
		 *
		 * @return const std::string&
		 */
		inline const std::string& path() {
			return fpath;
		}
	};

	inline data_type at(const std::string& _name) {
		return data_type(_name);
	}

} // namespace storage