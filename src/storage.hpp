#pragma once

#include <sys/stat.h>

#include <filesystem>
#include <string>

#include "common.hpp"
#include "flstream.hpp"

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
	Result<void*, Error> init();

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
		data_type(const std::string& _dname)
			: dname(_dname), fpath(DATA_DIR + dname) {
		}

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
		Result<void*, Error> remove();

		/**
		 * @brief Get metadata regarding a file.
		 *
		 * @return Result<meta, Error>
		 */
		Result<meta, Error> details();

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