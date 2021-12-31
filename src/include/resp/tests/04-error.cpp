#include <cstdint>
#include <cstring>

#include "catch.hpp"
#include "resp/resp.hpp"
#include "resp/types.hpp"

#define SIZE(x) sizeof(x) / sizeof(x[0])

TEST_CASE("resp::types::bulkstr tests", "[resp]") {
	const char *test_inputs[] = {
		"OK",
		"ERROR",
		"a very long string",
		"this\nhas\n\\rnewlines\\r\n",
	};

	SECTION("Test serialization & deserialization", "[resp]") {
		for (size_t i = 0; i < SIZE(test_inputs); i++) {
			char tmp[256] = {0};

			resp::types::error rstr = test_inputs[i];

			REQUIRE(rstr.length == strlen(test_inputs[i]));
			REQUIRE(strcmp(rstr.value, test_inputs[i]) == 0);

			// Test serialize() return value
			char *head = tmp;
			auto  size = rstr.serialize(head);
			REQUIRE((void *)(head - size) == (void *)tmp);

			// Test deserialization
			head = tmp;
			REQUIRE(strcmp(resp::types::error(head).value, test_inputs[i]) ==
					0);
		}
	}
}