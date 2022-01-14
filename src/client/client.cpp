#include "nio/ip/v4/client.hpp"

#include <chrono>
#include <iostream>
#include <string>
#include <string_view>
#include <thread>

#include "log.hpp"
#include "misc.hpp"
#include "resp.hpp"
#include "uid.hpp"

static int err(std::stringstream&, std::stringstream&, void*) {
	return 0;
}

static int ok(std::stringstream&, std::stringstream&, void*) {
	plog::v(LOG_INFO "parser", "Everything is ok");
	return 0;
}

static const resp::rcmd_t cmds[] = {{RESP_ERR_HANDLER, err}, {"OK", ok}};

static int get(void* _stream, const uid::uid_type& fid) {
	resp::parser parser(cmds);
	auto*		 stream = (nio::base::stream<sockaddr>*)_stream;

	// Send our command
	{
		std::stringstream command;
		resp::resps::simple_string("GET").put(command);
		resp::resps::simple_string(fid).put(command);
		stream->write(command.str().c_str(), command.str().length());
		DO_TIMEOUT;
	}

	// Read server file length
	int64_t fsize;
	{
		std::unique_ptr<char[]> buf(new char[MAX_NET_MSG_LEN + 1]);
		stream->read(buf.get(), MAX_NET_MSG_LEN);
		std::stringstream res {buf.get()};
		try {
			bool ok = false;
			std::visit(rediscpp::resp::detail::overloaded {
						   [&](resp::respds::simple_string const& val) {
							   if (val.get() == "OK")
								   ok = true;
						   },
						   [&](resp::respds::error_message const& val) {
							   plog::v(LOG_ERROR "server",
									   "Error: %s",
									   std::string(val.get()).c_str());
						   },
						   [&](auto const&) {
							   throw std::invalid_argument("invalid response");
						   }},
					   resp::value {res}.get());
			if (! ok)
				return -1;

			fsize = resp::value(res).as_integer();
		} catch (const std::exception& e) {
			plog::v(LOG_ERROR "get", "%s", e.what());
			return -1;
		}
	}

	{ // Start the file transfer
		std::stringstream res;
		resp::resps::simple_string("OK").put(res);
		stream->write(res.str().c_str(), res.str().length());
		DO_TIMEOUT;
	}

	// Read the file data
	std::string fdata;
	{
		std::unique_ptr<char[]> buf1(new char[MAX_NET_MSG_LEN + fsize + 1]);
		stream->read(buf1.get(), MAX_NET_MSG_LEN + fsize);
		std::stringstream res1 {buf1.get()};
		try {
			(void)resp::value {res1}.as_simple_string();
			fdata = std::string(resp::value {res1}.as_bulk_string());
		} catch (const std::exception& e) {
			plog::v(LOG_ERROR "server", "Invalid response");
			return -1;
		}
	}
	std::cout << "Received: \"" << fdata << "\"\n";

	return 0;
}

int main(int, char* argv[]) {
	nio::ip::v4::client cli(nio::ip::v4::addr("127.0.0.1", 8888));

	if (auto r = cli.Create()) {
		plog::v(LOG_ERROR "net", "Cannot create socket: %s", r.Err().msg);
		exit(r.Err().no);
	}

	nio::ip::v4::stream stream;

	if (auto r = cli.Connect()) {
		plog::v(LOG_ERROR "net", "Cannot connect: %s", r.Err().msg);
		exit(r.Err().no);
	} else
		stream = r.Ok();

	get(&stream, argv[1]);

	return 0;
}
