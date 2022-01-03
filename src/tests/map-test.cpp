#include "map.hpp"

#include <filesystem>
#include <fstream>
#include <string>

#include "catch.hpp"

static bool fcontains(const std::string& fpath, const char* needle) {
	std::string chunk;
	chunk.resize(256);
	std::fstream infile(fpath);
	while (! infile.eof()) {
		infile.read(chunk.data(), 255);
		if (chunk.find(needle) != std::string::npos) {
			infile.close();
			return true;
		}
	}
	return false;
}

static auto mappath =
	std::filesystem::temp_directory_path().string() + "/map-testing/";

static const char* objname = "testobj";

TEST_CASE("map tests", "[main]") {
	// Prepare directory and cd there
	std::filesystem::create_directory(mappath);
	std::filesystem::current_path(mappath);

	map::init();

	auto m = map::by_name(objname);

	auto id1 = m.create();
	auto id2 = m.create();

	SECTION("Test creation of objects", "[main]") {
		REQUIRE(m.exists(id1));
		REQUIRE(fcontains(mappath + MAP_ID_DIR + id1, objname));

		REQUIRE(m.exists(id2));
		REQUIRE(fcontains(mappath + MAP_ID_DIR + id2, objname));
	};

	SECTION("Test by_name()", "[main]") {
		auto s = map::by_name(objname);

		REQUIRE(s.name == m.name);
		for (size_t i = 0; i < m.ids.size(); i++) {
			REQUIRE(s.ids[i] == m.ids[i]);
		}
	}

	SECTION("Test by_id()", "[main]") {
		auto s = map::by_id(id1);

		REQUIRE(s.name == m.name);
		for (size_t i = 0; i < m.ids.size(); i++) {
			REQUIRE(s.ids[i] == m.ids[i]);
		}
	}

	SECTION("Test deletion of objects", "[main]") {
		m.remove(id1);

		REQUIRE(std::filesystem::exists(mappath + MAP_ID_DIR + id1) == false);
		REQUIRE(fcontains(mappath + MAP_NAME_DIR + objname, id1.c_str()) ==
				false);

		while (! m.ids.empty()) m.remove(m.ids[0]);

		REQUIRE(std::filesystem::exists(mappath + MAP_NAME_DIR + objname) ==
				false);
	}
}