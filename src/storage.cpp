#include "storage.hpp"

#include <cstdint>

// extern "C" {
//  I have found no other quick way and platform independant solution to do this
//  (not that the rest of the codebase is particularly platform independant)
#include <sys/stat.h>
//}

#include <filesystem>
#include <string>

namespace storage {

	meta::meta(const struct stat& _stat) {
		size  = _stat.st_size;
		atime = _stat.st_atim.tv_nsec / 1000 + _stat.st_atim.tv_sec * 1000000;
		mtime = _stat.st_mtim.tv_nsec / 1000 + _stat.st_mtim.tv_sec * 1000000;
	}

	Result<void*, Error> init() {
		Result<void*, Error> ret;
		try {
			std::filesystem::create_directory(STOR_DATA_DIR);
			return std::move(ret.Ok(nullptr));
		} catch (const std::filesystem::filesystem_error& e) {
			return std::move(ret.Err(e.code().value()));
		}
	}

	Result<void*, Error> data_type::save(const char* const buf, size_t len) {
		Result<void*, Error> ret;

		flstream stream((DATA_DIR + dname).c_str(),
						flstream::WRONLY | flstream::TRUNC | flstream::CREAT);

		if (stream.fail())
			return std::move(ret.Err(errno));

		stream.lock(flstream::ltype::WRITE);

		stream.write(buf, len);

		return std::move(ret.Ok(nullptr));
	}

	Result<void*, Error> data_type::remove() {
		Result<void*, Error> ret;
		try {
			std::filesystem::remove(fpath);
			return std::move(ret.Ok(nullptr));
		} catch (const std::filesystem::filesystem_error& e) {
			return std::move(ret.Err(e.code().value()));
		}
	}

	Result<meta, Error> data_type::details() {
		Result<meta, Error> ret;

		struct stat stbuf;
		if (stat(fpath.c_str(), &stbuf) < 0)
			return std::move(ret.Err(errno));

		return std::move(ret.Ok(stbuf));
	}
}; // namespace storage
