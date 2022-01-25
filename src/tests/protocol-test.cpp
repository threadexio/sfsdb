#include "protocol.hpp"

#include <cstdio>
#include <sstream>

#include "catch.hpp"
#include "messages.hpp"

#define LOG_BUFFER(x, l)                                                       \
	{                                                                          \
		printf(#x " = { ");                                                    \
		for (size_t i = 0; i < l; i++) {                                       \
			printf("0x%02x, ", (unsigned char)sdec[i]);                        \
		}                                                                      \
		printf("\b\b }\n");                                                    \
	}

TEST_CASE("protocol tests", "[main]") {
	SECTION("Test encoding & decoding", "[main]") {
		std::stringstream senc;

		protocol::types::invalid().to(senc);
		protocol::types::header(23, 123456789).to(senc);
		protocol::types::error("Test error message").to(senc);
		protocol::types::smallint(1234).to(senc);
		protocol::types::integer(5678).to(senc);
		protocol::types::string("Test string").to(senc);
		protocol::types::bigdata(9999).to(senc);

		// Check length
		REQUIRE(
			protocol::types::invalid::SIZE + protocol::types::header::SIZE +
				protocol::types::error::HEADER_SIZE +
				strlen("Test error message") + protocol::types::smallint::SIZE +
				protocol::types::integer::SIZE +
				protocol::types::string::HEADER_SIZE + strlen("Test string") +
				protocol::types::bigdata::HEADER_SIZE ==
			senc.str().length());

		std::string sdec_str = senc.str();
		auto*		sdec	 = sdec_str.c_str();

		REQUIRE(protocol::get_type(sdec) == protocol::types::ids::INVALID);
		LOG_BUFFER(sdec, protocol::types::invalid::SIZE);
		protocol::types::invalid().from(sdec);

		REQUIRE(protocol::get_type(sdec) == protocol::types::ids::HEADER);
		LOG_BUFFER(sdec, protocol::types::header::SIZE);
		protocol::types::header header;
		header.from(sdec);
		REQUIRE(header.command == 23);
		REQUIRE(header.length == 123456789);

		REQUIRE(protocol::get_type(sdec) == protocol::types::ids::ERROR);
		LOG_BUFFER(
			sdec,
			protocol::types::error::HEADER_SIZE + strlen("Test error message"));
		protocol::types::error err;
		err.from(sdec);
		REQUIRE(strcmp(err.msg, "Test error message") == 0);

		REQUIRE(protocol::get_type(sdec) == protocol::types::ids::SMALLINT);
		LOG_BUFFER(sdec, protocol::types::smallint::SIZE);
		protocol::types::smallint smallint;
		smallint.from(sdec);
		REQUIRE(smallint.val == 1234);

		REQUIRE(protocol::get_type(sdec) == protocol::types::ids::INTEGER);
		LOG_BUFFER(sdec, protocol::types::integer::SIZE);
		protocol::types::integer integer;
		integer.from(sdec);
		REQUIRE(integer.val == 5678);

		REQUIRE(protocol::get_type(sdec) == protocol::types::ids::STRING);
		LOG_BUFFER(
			sdec, protocol::types::string::HEADER_SIZE + strlen("Test string"));
		protocol::types::string str;
		str.from(sdec);
		REQUIRE(str.str == "Test string");

		REQUIRE(protocol::get_type(sdec) == protocol::types::ids::BIGDATA);
		LOG_BUFFER(sdec, protocol::types::bigdata::HEADER_SIZE);
		protocol::types::bigdata bigdata;
		bigdata.from(sdec);
		REQUIRE(bigdata.length == 9999);
	}
}