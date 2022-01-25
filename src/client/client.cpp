#include "nio/ip/v4/client.hpp"

#include <fstream>
#include <iostream>
#include <memory>

#include "log.hpp"
#include "misc.hpp"
#include "protocol.hpp"
#include "uid.hpp"

static int get(void* _stream, const uid::uid_type& id) {
	auto* stream = (nio::base::stream<sockaddr>*)_stream;

	{
		// Send the command
		std::stringstream		tmp;
		protocol::types::string fid(id.c_str());
		protocol::types::header head(
			1, protocol::types::string::HEADER_SIZE + fid.length);

		head.to(tmp);
		fid.to(tmp);

		if (auto r = stream->write(tmp.str().c_str(), tmp.str().length())) {
			plog::v(LOG_ERROR "net", "Cannot write: %s", r.Err().msg);
			return r.Err().no;
		}
	}

	// Read response
	{
		// Read message header
		protocol::types::header head;
		{
			char tmp[protocol::types::header::SIZE];
			stream->read(tmp, sizeof(tmp), MSG_WAITALL);
			const char* tmp1 = tmp;
			head.from(tmp1);
		}

		if (head.type != protocol::types::ids::HEADER) {
			plog::v(LOG_ERROR "client", "Bad response");
			return -1;
		}

		std::unique_ptr<char[]> _req(new char[head.length + 1]);
		if (auto r = stream->read(_req.get(), head.length)) {
			plog::v(LOG_ERROR "net", "Cannot read: %s", r.Err().no);
			return r.Err().no;
		}
		const char* req = _req.get();

		if (head.command != protocol::status::SUCCESS) {
			protocol::types::error err;
			err.from(req);
			plog::v(LOG_ERROR "get", "Error: %s", err.msg);
			return -1;
		}

		// read response data
		protocol::types::bigdata fdata;
		fdata.from(req);

		plog::v(LOG_INFO "get", "Received %u bytes: \"%s\"", fdata.length, req);
	}
	return 0;
}

static int put(void*			  _stream,
			   const std::string& filename,
			   const std::string& filepath) {
	auto* stream = (nio::base::stream<sockaddr>*)_stream;

	{
		// Construct the command
		std::stringstream tmp;

		protocol::types::header head(2,
									 protocol::types::string::HEADER_SIZE +
										 protocol::types::bigdata::HEADER_SIZE);

		protocol::types::string fname(filename.c_str());
		head.length += fname.length;

		std::fstream fstr(filepath,
						  std::ios::in | std::ios::ate | std::ios::binary);
		size_t		 filesize = fstr.tellg();
		fstr.seekg(0, std::ios::beg);

		protocol::types::bigdata fdata(filesize);
		head.length += filesize;

		head.to(tmp);
		fname.to(tmp);
		fdata.to(tmp);

		if (auto r = stream->write(
				tmp.str().c_str(), tmp.str().length(), MSG_MORE)) {
			plog::v(LOG_ERROR "net", "Cannot write: %s", r.Err().msg);
			return r.Err().no;
		}

		size_t					written_bytes = 0;
		std::unique_ptr<char[]> blk(new char[4096]);
		while (written_bytes < fdata.length) {
			auto bytes_to_write = (fdata.length - written_bytes > 4096)
									  ? 4096
									  : fdata.length - written_bytes;

			fstr.read(blk.get(), bytes_to_write);

			if (auto r = stream->write(blk.get(), bytes_to_write, MSG_MORE)) {
				plog::v(LOG_ERROR "net", "Cannot write: %s", r.Err().msg);
				return r.Err().no;
			}

			written_bytes += 4096;
		}

		// Finalize send
		stream->write("", 0);
	}

	{
		// Read the response

		// Read message header
		protocol::types::header head;
		{
			char buf[protocol::types::header::SIZE];
			if (auto r = stream->read(buf, sizeof(buf))) {
				plog::v(LOG_ERROR "net", "Cannnot read: %s", r.Err().msg);
				return r.Err().no;
			}

			const char* tmp = buf;
			head.from(tmp);

			if (head.type != protocol::types::ids::HEADER) {
				plog::v(LOG_ERROR "client", "Bad response");
				return -1;
			}
		}

		std::unique_ptr<char[]> _req(new char[head.length + 1]);
		if (auto r = stream->read(_req.get(), head.length)) {
			plog::v(LOG_ERROR "net", "Cannot read: %s", r.Err().no);
			return r.Err().no;
		}
		const char* req = _req.get();

		if (head.command != protocol::status::SUCCESS) {
			protocol::types::error err;
			err.from(req);
			plog::v(LOG_ERROR "server", "Error: %s", err.msg);
			return -1;
		}

		// Read response data
		protocol::types::string fid;
		if (protocol::get_type(req, fid)) {
			plog::v(LOG_ERROR "client", "Expected file id");
			return -1;
		}

		plog::v(LOG_NOTICE "put", "File id: %s", fid.str.c_str());

		return 0;
	}
}

static int desc(void* _stream, const uid::uid_type& id) {
	auto* stream = (nio::base::stream<sockaddr>*)_stream;

	{
		std::stringstream tmp;

		protocol::types::header(
			3, protocol::types::string::HEADER_SIZE + id.length())
			.to(tmp);
		protocol::types::string(id.c_str()).to(tmp);

		if (auto r = stream->write(tmp.str().c_str(), tmp.str().length())) {
			plog::v(LOG_ERROR "net", "Cannnot write: %s", r.Err().msg);
			return r.Err().no;
		}
	}

	{
		// Read message header
		protocol::types::header head;
		{
			char buf[protocol::types::header::SIZE];
			if (auto r = stream->read(buf, sizeof(buf))) {
				plog::v(LOG_ERROR "net", "Cannnot read: %s", r.Err().msg);
				return r.Err().no;
			}

			const char* tmp = buf;
			head.from(tmp);

			if (head.type != protocol::types::ids::HEADER) {
				plog::v(LOG_ERROR "client", "Bad response");
				return -1;
			}
		}

		std::unique_ptr<char[]> _req(new char[head.length + 1]);
		if (auto r = stream->read(_req.get(), head.length)) {
			plog::v(LOG_ERROR "net", "Cannot read: %s", r.Err().no);
			return r.Err().no;
		}
		const char* req = _req.get();

		if (head.command != protocol::status::SUCCESS) {
			protocol::types::error err;
			err.from(req);
			plog::v(LOG_ERROR "server", "Error: %s", err.msg);
			return -1;
		}

		// Read the response data
		protocol::types::string fname;
		if (protocol::get_type(req, fname)) {
			plog::v(LOG_ERROR "client", "Expected file name");
			return -1;
		}
		fname.from(req);

		protocol::types::integer fsize;
		if (protocol::get_type(req, fsize)) {
			plog::v(LOG_ERROR "client", "Expected file size");
			return -1;
		}
		fsize.from(req);

		plog::v(LOG_NOTICE "client",
				"Name: %s, Size: %u",
				fname.str.c_str(),
				fsize.val);
	}
	return 0;
}

static int del(void* _stream, const uid::uid_type& id) {
	auto* stream = (nio::base::stream<sockaddr>*)_stream;

	{
		std::stringstream tmp;

		protocol::types::header(
			4, protocol::types::string::HEADER_SIZE + id.length())
			.to(tmp);
		protocol::types::string(id.c_str()).to(tmp);

		if (auto r = stream->write(tmp.str().c_str(), tmp.str().length())) {
			plog::v(LOG_ERROR "net", "Cannnot write: %s", r.Err().msg);
			return r.Err().no;
		}
	}

	{
		// Read message header
		protocol::types::header head;
		{
			char buf[protocol::types::header::SIZE];
			if (auto r = stream->read(buf, sizeof(buf))) {
				plog::v(LOG_ERROR "net", "Cannnot read: %s", r.Err().msg);
				return r.Err().no;
			}

			const char* tmp = buf;
			head.from(tmp);

			if (head.type != protocol::types::ids::HEADER) {
				plog::v(LOG_ERROR "client", "Bad response");
				return -1;
			}
		}

		std::unique_ptr<char[]> _req(new char[head.length + 1]);
		if (auto r = stream->read(_req.get(), head.length)) {
			plog::v(LOG_ERROR "net", "Cannot read: %s", r.Err().no);
			return r.Err().no;
		}
		const char* req = _req.get();

		if (head.command != protocol::status::SUCCESS) {
			protocol::types::error err;
			err.from(req);
			plog::v(LOG_ERROR "server", "Error: %s", err.msg);
			return -1;
		}
		return 0;
	}
}

int main(int argc, char* argv[]) {
	if (argc < 2) {
		std::cerr << "Usage: " << argv[0] << " command [command args...]\n";
		return EXIT_FAILURE;
	}

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

	if (strcmp("get", argv[1]) == 0) {
		return get(&stream, argv[2]);
	} else if (strcmp("put", argv[1]) == 0) {
		return put(&stream, argv[2], argv[3]);
	} else if (strcmp("desc", argv[1]) == 0) {
		return desc(&stream, argv[2]);
	} else if (strcmp("del", argv[1]) == 0) {
		return del(&stream, argv[2]);
	} else {
		plog::v(LOG_ERROR "client", "Unknown command: %s", argv[1]);
		return EXIT_FAILURE;
	}

	return 0;
}
