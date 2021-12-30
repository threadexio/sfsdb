#include <cstring>

#include "catch.hpp"
#include "nio/buffer.hpp"

TEST_CASE("nio::buffer tests", "[nio]") {
	const char* testdata	 = "abcdefghijk\n\r!";
	size_t		testdata_len = strlen(testdata);

	nio::buffer buffer((void*)testdata, testdata_len);

	// Test initial constructor
	REQUIRE(buffer.length() == testdata_len);
	REQUIRE(strncmp((const char*)buffer.data(), testdata, testdata_len) == 0);

	SECTION("Comparing by index") {
		REQUIRE(buffer.at(0) == testdata[0]);
		REQUIRE(buffer.at(1) == testdata[1]);
		REQUIRE(buffer.at(2) == testdata[2]);
	}

	SECTION("Clearing data") {
		buffer.clear();

		REQUIRE(buffer.length() == 0);
		REQUIRE(buffer.empty());
	}

	SECTION("Reading & Writing data") {
		// Add offset at the start just to also test .seek()
		buffer.write((long long)23);
		buffer.seek(sizeof(long long));

		struct test_t {
			int			 a	   = 0;
			unsigned int b	   = 0;
			double		 c	   = 42.45;
			char		 d[25] = {0};
		};

		test_t testobj = {};
		testobj.a	   = 10;
		testobj.b	   = 345;
		strcpy(testobj.d, "hello there!\n");

		size_t write_pos = buffer.write(testobj);

		REQUIRE(buffer.length() == sizeof(long long) + sizeof(testobj));

		buffer.seek(write_pos);

		test_t readobj = buffer.read<test_t>();

		REQUIRE(readobj.a == testobj.a);
		REQUIRE(readobj.b == testobj.b);
		REQUIRE(readobj.c == testobj.c);
		REQUIRE(strncmp(readobj.d, testobj.d, sizeof(test_t::d)) == 0);
	}
}