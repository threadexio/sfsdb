#include "storage.hpp"

#include <cstdint>

extern "C" {
// I have found no other quick way and platform independant solution to do this
// (not that the rest of the codebase is particularly platform independant)
#include <sys/stat.h>
}

#include <filesystem>
#include <fstream>
#include <string>

#define STOR_DATA_DIR "data/"

namespace storage {

	meta::meta(const struct stat& _stat) {
		size  = _stat.st_size;
		atime = _stat.st_atim.tv_nsec / 1000 + _stat.st_atim.tv_sec * 1000000;
		mtime = _stat.st_mtim.tv_nsec / 1000 + _stat.st_mtim.tv_sec * 1000000;
	}

	data_type::data_type(const std::string& _dname) {
		dname = _dname;
		stream.open(STOR_DATA_DIR + _dname, std::ios::in | std::ios::binary);
	}

	data_type::~data_type() {
		stream.close();
	}

	void data_type::save(const char* const buf, size_t len) {
		stream.close();
		stream.open(STOR_DATA_DIR + dname,
					std::ios::out | std::ios::trunc | std::ios::binary);
		stream.write(buf, len);
		stream.close();

		stream.open(STOR_DATA_DIR + dname, std::ios::in);
	}
}; // namespace storage
