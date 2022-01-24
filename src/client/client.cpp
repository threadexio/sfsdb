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
		std::stringstream command;
		protocol::types::header(
			1, protocol::types::string::DATA_HEADER_SIZE + fid.length())
			.to(command);
		protocol::types::string(fid.c_str()).to(command);

		if (auto r =
				stream->write(command.str().c_str(), command.str().length())) {
			plog::v(LOG_ERROR "net", "Cannot write: %s", r.Err().msg);
			return r.Err().no;
		}
	}

	// read response message header
	protocol::types::header head;
	{
		char tmp[protocol::types::header::SIZE];
		stream->read(tmp, sizeof(tmp), MSG_WAITALL);
		const char* tmp1 = tmp;
		head.from(tmp1);
	}

	if (head.type != protocol::types::ids::HEADER) {
		plog::v(LOG_ERROR "server", "Bad response");
		return -1;
	}

	std::unique_ptr<char[]> buf(new char[head.length + 1]);
	if (auto r = stream->read(buf.get(), head.length, MSG_WAITALL)) {
		plog::v(LOG_ERROR "net", "Cannot read: %s", r.Err().msg);
		return r.Err().no;
	}
	buf.get()[head.length] = 0;

	// read response data
	protocol::types::bigdata fdata_header;
	const char*				 tmp = buf.get();
	fdata_header.from(tmp);

	if (head.command == 0) {
		plog::v(LOG_INFO "get",
				"Received %u bytes: \"%s\"",
				fdata_header.length,
				tmp);
	} else {
		plog::v(LOG_ERROR "get", "Error: %s", tmp);
	}

	return 0;
}

int main(int, char* argv[]) {
	nio::ip::v4::client cli(nio::ip::v4::addr("127.0.0.1", 8889));

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
