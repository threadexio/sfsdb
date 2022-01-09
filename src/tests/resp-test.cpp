#include "resp.hpp"

#include <stdexcept>

#include "catch.hpp"

static int invalid(std::stringstream &) {
	return -1;
}

static int error(std::stringstream &) {
	return -2;
}

static int test1(std::stringstream &) {
	return 0;
}

static int test2(std::stringstream &data) {
	int ret = -1;
	std::visit(rediscpp::resp::detail::overloaded {
				   [&](resp::respds::bulk_string const &) { ret = 0; },
				   [&](auto const &) { ret = -1; }},
			   resp::value {data}.get());
	return ret;
}

static int test3(std::stringstream &data) {
	try {
		int ret = -1;

		int64_t		arg1;
		std::string arg2;

		std::visit(
			rediscpp::resp::detail::overloaded {
				[&](resp::respds::integer const &val) { arg1 = val.get(); },
				[&](auto const &) {
					throw std::invalid_argument("invalid arg1");
				}},
			resp::value {data}.get());

		std::visit(
			rediscpp::resp::detail::overloaded {
				[&](resp::respds::bulk_string const &val) { arg2 = val.get(); },
				[&](auto const &) {
					throw std::invalid_argument("invalid arg2");
				}},
			resp::value {data}.get());

		if (arg2 == "argument2" && arg1 == 1337)
			ret = 0;

		return ret;
	} catch (const std::exception &e) { return -1; }
}

static const resp::rcmd_t commands[] = {{RESP_INV_HANDLER, invalid},
										{RESP_ERR_HANDLER, error},
										{"TEST1", test1},
										{"TEST2", test2},
										{"TEST3", test3}};

static resp::parser parser(commands);

TEST_CASE("rediscpp wrapper tests", "[main]") {
	SECTION("Test command \"TEST1\"", "[main]") {
		std::stringstream ss;

		resp::resps::put(ss, resp::resps::simple_string {"TEST1"});
		resp::resps::put(ss, resp::resps::bulk_string {"extra data"});

		REQUIRE(parser.parse(ss) == 0);
	}

	SECTION("Test command \"TEST2\"", "[main]") {
		std::stringstream ss;

		resp::resps::put(ss, resp::resps::simple_string {"TEST2"});
		resp::resps::put(ss, resp::resps::bulk_string {"extra data"});

		REQUIRE(parser.parse(ss) == 0);
	}

	SECTION("Test command \"TEST3\"", "[main]") {
		std::stringstream ss;

		resp::resps::put(ss, resp::resps::simple_string {"TEST3"});
		resp::resps::put(ss, resp::resps::integer {1337});
		resp::resps::put(ss, resp::resps::bulk_string {"argument2"});

		REQUIRE(parser.parse(ss) == 0);
	}

	SECTION("Test error handler", "[main]") {
		std::stringstream ss;

		resp::resps::put(
			ss, resp::resps::error_message {"a meaningful error message"});
		resp::resps::put(ss, resp::resps::integer {-42});

		REQUIRE(parser.parse(ss) == -2);
	}

	SECTION("Test invalid command/format handler", "[main]") {
		std::stringstream ss;

		resp::resps::put(ss, resp::resps::bulk_string {"random data"});
		resp::resps::put(ss, resp::resps::integer {42});

		REQUIRE(parser.parse(ss) == -1);
	}
}