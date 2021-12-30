#include <asm-generic/errno-base.h>
#include <errno.h>

#include <cstring>

#include "catch.hpp"
#include "nio/error.hpp"

#define TEST_ERRNO(x)                                                          \
	{                                                                          \
		SECTION("Testing error " #x, "[nio]") {                                \
			error.set(x);                                                      \
			REQUIRE(error.err == x);                                           \
			REQUIRE(error.msg == strerror(x));                                 \
		}                                                                      \
	}

TEST_CASE("nio::error tests", "[nio]") {
	nio::error error(EPERM);
	REQUIRE(error.err == EPERM);
	REQUIRE(error.msg == strerror(EPERM));

	TEST_ERRNO(ENOENT);
	TEST_ERRNO(ESRCH);
	TEST_ERRNO(EINTR);
}