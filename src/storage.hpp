#pragma once

extern "C" {
#include <sys/stat.h>
}

#include <filesystem>
#include <fstream>
#include <string>

#define STOR_DATA_DIR "data/"

namespace storage {

	struct meta {
		// Last access time (microseconds)
		uint64_t atime = 0;
		// Last modification time (microseconds)
		uint64_t mtime = 0;
		// Size of file
		uint64_t size = 0;

		meta(const struct stat& _stat);
	};

	/**
	 * @brief Create directory structure @ cwd.
	 *
	 */
	inline void init() {
		std::filesystem::create_directory(STOR_DATA_DIR);
	}

	struct data_type {
	private:
		std::string	 dname;
		std::fstream stream;

	public:
		/**
		 * @brief Open or create a new data object at _dname.
		 *
		 * @param _dname
		 */
		data_type(const std::string& _dname);

		~data_type();

		/**
		 * @brief Save data in the object. Will overwrite previous data!
		 *
		 * @param buf Buffer containing the data
		 * @param len Length of the data
		 */
		void save(const char* const buf, size_t len);

		/**
		 * @brief Remove the current data.
		 *
		 */
		inline void remove() {
			stream.close();
			std::filesystem::remove(STOR_DATA_DIR + dname);
		}

		inline meta details() {
			struct stat stbuf;
			stat((STOR_DATA_DIR + dname).c_str(), &stbuf);
			return stbuf;
		}

		/**
		 * @brief Get a reference to the data stream.
		 *
		 * @return std::fstream&
		 */
		inline std::fstream& get() {
			return stream;
		}
	};

	inline data_type at(const std::string& _name) {
		return data_type(_name);
	}

} // namespace storage