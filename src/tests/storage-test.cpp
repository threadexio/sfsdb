#include "storage.hpp"

#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>

#include "catch.hpp"
#include "cfg.hpp"

static const char* dataname = "some_random_name";

static const char* data1 = "very important data\n";
static const char* data2 = "some\r\nother very important data\n";

static std::string read_data(const std::string& path) {
	char* buf = new char[256];
	memset(buf, 0, 256);

	std::fstream infile(STOR_DATA_DIR + path, std::ios::in);
	infile.read(buf, 255);
	infile.close();

	std::string ret = buf;
	delete[] buf;
	return ret;
}

TEST_CASE("storage tests", "[main]") {
	// Prepare directory and cd there
	std::filesystem::create_directory(testpath);
	std::filesystem::current_path(testpath);

	if (auto r = storage::init())
		LOG_ERROR(r.Err().msg)

	auto obj = storage::at(dataname);

	SECTION("Test creation of data", "[main]") {
		if (auto r = obj.save(data2, strlen(data2)))
			LOG_ERROR(r.Err().msg)

		REQUIRE(read_data(dataname) == data2);
	}

	SECTION("Test overwrite of data", "[main]") {
		if (auto r = obj.save(data1, strlen(data1)))
			LOG_ERROR(r.Err().msg)

		REQUIRE(read_data(dataname) == data1);
	}

	SECTION("Test read pf data", "[main]") {
	}

	SECTION("Test details()", "[main]") {
		struct stat stbuf;

		// Ah the wonders of c++, cast it to std::string, concat it with another
		// string, and then back to const char*.
		stat((std::string(STOR_DATA_DIR) + dataname).c_str(), &stbuf);

		storage::meta _details;
		if (auto r = obj.details())
			LOG_ERROR(r.Err().msg)
		else
			_details = r.Ok();

		REQUIRE(_details.size == (uint64_t)stbuf.st_size);
		REQUIRE(_details.atime == (stbuf.st_atim.tv_nsec / 1000ULL +
								   stbuf.st_atim.tv_sec * 1000000ULL));
		REQUIRE(_details.mtime == (stbuf.st_mtim.tv_nsec / 1000ULL +
								   stbuf.st_mtim.tv_sec * 1000000ULL));
	}

	SECTION("Test deletion of data", "[main]") {
		obj.remove();

		REQUIRE(std::filesystem::exists(STOR_DATA_DIR +
										std::string(dataname)) == false);
	}
}