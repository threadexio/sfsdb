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
		senc << protocol::MAGIC;

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

		protocol::types::invalid invalid;
		REQUIRE(protocol::get_type(sdec, invalid).no == 0);
		LOG_BUFFER(sdec, protocol::types::invalid::SIZE);

		protocol::types::header header;
		REQUIRE(protocol::get_type(sdec, header).no == 0);
		LOG_BUFFER(sdec, protocol::types::header::SIZE);
		REQUIRE(header.command == 23);
		REQUIRE(header.length == 123456789);

		protocol::types::error err;
		REQUIRE(protocol::get_type(sdec, err).no == 0);
		LOG_BUFFER(
			sdec,
			protocol::types::error::HEADER_SIZE + strlen("Test error message"));
		REQUIRE(strcmp(err.msg, "Test error message") == 0);

		protocol::types::smallint smallint;
		REQUIRE(protocol::get_type(sdec, smallint).no == 0);
		LOG_BUFFER(sdec, protocol::types::smallint::SIZE);
		REQUIRE(smallint.val == 1234);

		protocol::types::integer integer;
		REQUIRE(protocol::get_type(sdec, integer).no == 0);
		LOG_BUFFER(sdec, protocol::types::integer::SIZE);
		REQUIRE(integer.val == 5678);

		protocol::types::string str;
		REQUIRE(protocol::get_type(sdec, str).no == 0);
		LOG_BUFFER(
			sdec, protocol::types::string::HEADER_SIZE + strlen("Test string"));
		REQUIRE(str.str == "Test string");

		protocol::types::bigdata bigdata;
		REQUIRE(protocol::get_type(sdec, bigdata).no == 0);
		LOG_BUFFER(sdec, protocol::types::bigdata::HEADER_SIZE);
		REQUIRE(bigdata.length == 9999);
	}
}