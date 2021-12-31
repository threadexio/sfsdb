#include <cstdint>
#include <cstring>

#include "catch.hpp"
#include "resp/resp.hpp"
#include "resp/types.hpp"

TEST_CASE("Multiple type tests", "[resp]") {
	resp::types::integer v1 = 12345;
	resp::types::simstr	 v2 = "hello there\n";
	resp::types::bulkstr v3 = "a very big string\r\n with lots of data\n\r";
	resp::types::error	 v4 = "A meaningful\nerror message";

	SECTION("Test serialization & deserialization for multiple types",
			"[resp]") {
		char tmp[1024];

		size_t size = 0;

		// Serialize everything
		char* head = tmp;
		size += v1.serialize(head);
		size += v2.serialize(head);
		size += v3.serialize(head);
		size += v4.serialize(head);

		REQUIRE((void*)(head - size) == (void*)tmp);

		head = tmp;
		REQUIRE(resp::types::integer(head).value == v1.value);
		REQUIRE(strcmp(resp::types::simstr(head).value, v2.value) == 0);
		REQUIRE(strcmp(resp::types::bulkstr(head).value, v3.value) == 0);
		REQUIRE(strcmp(resp::types::error(head).value, v4.value) == 0);
	}
}