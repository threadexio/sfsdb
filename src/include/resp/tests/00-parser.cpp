#include <stdio.h>

#include <cstdint>
#include <cstring>

#include "catch.hpp"
#include "resp/resp.hpp"
#include "resp/types.hpp"

static int c1(char* data, void*) {
	return resp::types::integer(data).value;
}

static int c2(char* data, void*) {
	return strcmp(resp::types::bulkstr(data).value,
				  "this is the\r\n expected string\nwith many newlines");
}

static int error(char* data, void*) {
	auto err  = resp::types::error(data);
	auto code = resp::types::integer(data);
	return -code.value;
}

static int invalid(char*, void*) {
	return 0;
}

static const resp::rcmd_t commands[] = {
	{"C1", c1}, {"C2", c2}, {"_ERR", error}, {"_INV", invalid}};

TEST_CASE("resp::parser tests", "[resp]") {
	resp::parser parser = commands;

	SECTION("Testing command \"C1\"", "[resp]") {
		char  tmp[1024];
		char* head = tmp;

		resp::types::simstr("C1").serialize(head);
		resp::types::integer(-234235).serialize(head);

		auto result = parser.parse(tmp, nullptr);
		REQUIRE(result == -234235);
	}

	SECTION("Testing command \"C2\"", "[resp]") {
		char  tmp[1024];
		char* head = tmp;

		resp::types::simstr("C2").serialize(head);
		resp::types::bulkstr(
			"this is the\r\n expected string\nwith many newlines")
			.serialize(head);

		auto result = parser.parse(tmp, nullptr);
		REQUIRE(result == 0);
	}

	SECTION("Testing error handler", "[resp]") {
		char  tmp[1024];
		char* head = tmp;

		resp::types::error("Some error message").serialize(head);
		resp::types::integer(64).serialize(head);

		auto result = parser.parse(tmp, nullptr);
		REQUIRE(result == -64);
	}

	SECTION("Testing invalid command handler", "[resp]") {
		char  tmp[1024];
		char* head = tmp;

		resp::types::simstr("Some other command").serialize(head);
		resp::types::bulkstr("this wont be\n\rprocessed").serialize(head);

		auto result = parser.parse(tmp, nullptr);
		REQUIRE(result == 0);
	}
}