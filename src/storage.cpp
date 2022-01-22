#include "storage.hpp"

#include <cstdint>

// extern "C" {
//  I have found no other quick way and platform independant solution to do this
//  (not that the rest of the codebase is particularly platform independant)
#include <sys/stat.h>
//}

#include <filesystem>
#include <fstream>
#include <string>

namespace storage {

	meta::meta(const struct stat& _stat) {
		size  = _stat.st_size;
		atime = _stat.st_atim.tv_nsec / 1000 + _stat.st_atim.tv_sec * 1000000;
		mtime = _stat.st_mtim.tv_nsec / 1000 + _stat.st_mtim.tv_sec * 1000000;
	}

	data_type::data_type(const std::string& _dname)
		: dname(_dname), fpath(DATA_DIR + dname) {
	}

	Result<void*, Error> data_type::save(const char* const buf, size_t len) {
		Result<void*, Error> ret;

		std::fstream stream(DATA_DIR + dname,
							std::ios::out | std::ios::trunc | std::ios::binary);
		if (stream.fail())
			return std::move(ret.Err(errno));

		stream.write(buf, len);
		stream.close();

		stream.open(DATA_DIR + dname, std::ios::in);
		if (stream.fail())
			return std::move(ret.Err(errno));

		return std::move(ret.Ok(nullptr));
	}
}; // namespace storage
