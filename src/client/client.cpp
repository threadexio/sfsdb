#include "nio/ip/v4/client.hpp"

#include <iostream>

#include "log.hpp"
#include "misc.hpp"
#include "protocol.hpp"
#include "uid.hpp"

static int get(void* _stream, const uid::uid_type& fid) {
	auto* stream = (nio::base::stream<sockaddr>*)_stream;

	{
		// Send the command
		char tmp[protocol::HEADER_SIZE];
		protocol::header {16 /* GET command, as defined in server.cpp */, 32}
			.to(tmp);
		stream->write(tmp, sizeof(tmp), MSG_MORE);
		stream->write(fid.data(), 32);
	}

	// read response
	char tmp[protocol::HEADER_SIZE];
	stream->read(tmp, protocol::HEADER_SIZE, MSG_WAITALL);
	auto head = protocol::header::from(tmp);

	std::unique_ptr<char[]> buf(new char[head.length + 1]);
	stream->read(buf.get(), head.length, MSG_WAITALL);
	buf.get()[head.length] = 0;

	if (head.command == protocol::SUCCESS_HANDLER) {
		plog::v(LOG_INFO "get",
				"Received %u bytes: \"%s\"",
				head.length,
				buf.get());
	} else {
		plog::v(LOG_ERROR "get", "Error: %s", buf.get());
	}

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
