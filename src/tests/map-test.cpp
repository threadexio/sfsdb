#include "map.hpp"

#include <filesystem>
#include <fstream>
#include <string>

#include "catch.hpp"
#include "cfg.hpp"

static bool fcontains(const std::string& fpath, const char* needle) {
	std::string chunk;
	chunk.resize(256);
	std::fstream infile(fpath);

	if (infile.fail())
		return false;

	while (! infile.eof()) {
		infile.read(chunk.data(), 255);
		if (chunk.find(needle) != std::string::npos) {
			infile.close();
			return true;
		}
	}
	return false;
}

static const char* objname = "testobj";

TEST_CASE("map tests", "[main]") {
	// Prepare directory and cd there
	std::filesystem::create_directory(testpath);
	std::filesystem::current_path(testpath);

	if (auto r = map::init())
		LOG_ERROR(r.Err().msg)

	auto m = map::by_name(objname);

	uid::uid_type id1;
	if (auto r = m.create())
		LOG_ERROR(r.Err().msg)
	else
		id1 = r.Ok();

	uid::uid_type id2;
	if (auto r = m.create())
		LOG_ERROR(r.Err().msg)
	else
		id2 = r.Ok();

	SECTION("Test creation of objects", "[main]") {
		REQUIRE(m.exists(id1));
		REQUIRE(fcontains(testpath + map::ID_DIR + id1, objname));

		REQUIRE(m.exists(id2));
		REQUIRE(fcontains(testpath + map::ID_DIR + id2, objname));
	};

	SECTION("Test by_name()", "[main]") {
		auto s = map::by_name(objname);

		REQUIRE(s.name == m.name);
		for (size_t i = 0; i < m.ids.size(); i++) {
			REQUIRE(s.ids[i] == m.ids[i]);
		}
	}

	SECTION("Test by_id()", "[main]") {
		map::map_type s;

		if (auto r = map::by_id(id1))
			LOG_ERROR(r.Err().msg)
		else
			s = r.Ok();

		REQUIRE(s.name == m.name);
		for (size_t i = 0; i < m.ids.size(); i++) {
			REQUIRE(s.ids[i] == m.ids[i]);
		}
	}

	SECTION("Test deletion of objects", "[main]") {
		if (auto r = m.remove(id1))
			LOG_ERROR(r.Err().msg)

		REQUIRE(std::filesystem::exists(testpath + map::ID_DIR + id1) == false);
		REQUIRE(fcontains(testpath + map::NAME_DIR + objname, id1.c_str()) ==
				false);

		while (! m.ids.empty())
			if (auto r = m.remove(m.ids[0]))
				LOG_ERROR(r.Err().msg)

		REQUIRE(std::filesystem::exists(testpath + map::NAME_DIR + objname) ==
				false);
	}
}