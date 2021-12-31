#include <cstdint>

#include "catch.hpp"
#include "resp/resp.hpp"
#include "resp/types.hpp"

#define SIZE(x) sizeof(x) / sizeof(x[0])

TEST_CASE("resp::types::integer tests", "[resp]") {
	const int64_t test_inputs[] = {
		123456789,
		-123456789,
		INT64_MAX,
		INT64_MIN,
	};

	SECTION("Test serialization & deserialization", "[resp]") {
		for (size_t i = 0; i < SIZE(test_inputs); i++) {
			char tmp[256] = {0};

			resp::types::integer rint = test_inputs[i];

			REQUIRE(rint.value == test_inputs[i]);

			// Test serialize() return value
			char *head = tmp;
			auto  size = rint.serialize(head);
			REQUIRE((void *)(head - size) == (void *)tmp);

			// Test deserialization
			head = tmp;
			REQUIRE(resp::types::integer(head).value == test_inputs[i]);
		}
	}
}