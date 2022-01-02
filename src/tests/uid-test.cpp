#include "uid.hpp"

#include <array>
#include <chrono>
#include <cstdint>
#include <cstring>

#include "catch.hpp"

TEST_CASE("uid generator tests", "[main]") {
	uid::generator uidgen;

	SECTION("Test uid format") {
		auto cur = uidgen.get();
		REQUIRE(cur.length() == UID_TOTAL_LEN);
	}

	SECTION("Test uid uniqueness", "[main]") {
		const size_t samples = 10000;

		auto prev = uidgen.get();
		for (size_t i = 0; i < samples; i++) {
			auto cur = uidgen.get();
			REQUIRE(prev != cur);
			prev = cur;
		}
	}
}