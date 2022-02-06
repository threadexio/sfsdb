#include "nio/ip/v4/client.hpp"

#include <boost/program_options.hpp>
#include <csignal>
#include <fstream>
#include <iostream>
#include <memory>
#include <vector>

#include "log.hpp"
#include "misc.hpp"
#include "nio/ip/stream.hpp"
#include "protocol.hpp"
#include "uid.hpp"

namespace po = boost::program_options;

static nio::ip::v4::stream stream;

static int get(const uid::uid_type& id) {
	{
		// Send the command
		std::stringstream		tmp;
		protocol::types::string fid(id.c_str());
		protocol::types::header head(
			1, protocol::types::string::HEADER_SIZE + fid.length);

		head.to(tmp);
		fid.to(tmp);

		if (auto r = stream.write(tmp.str().c_str(), tmp.str().length())) {
			plog::v(LOG_ERROR "net", "Cannot write: %s", r.Err().msg);
			return r.Err().no;
		}
	}

	// Read response
	{
		// Read message header
		protocol::types::header head;
		{
			char buf[protocol::types::header::SIZE];
			if (auto r = stream.read(buf, sizeof(buf), MSG_WAITALL)) {
				plog::v(LOG_ERROR "net", "Cannnot read: %s", r.Err().msg);
				return r.Err().no;
			}
			const char* tmp = buf;

			if (auto err = protocol::get_type(tmp, head)) {
				plog::v(LOG_ERROR "client", "Bad response");
				return -1;
			}
		}

		std::unique_ptr<char[]> _req(new char[head.length + 1]);
		if (auto r = stream.read(_req.get(), head.length, MSG_WAITALL)) {
			plog::v(LOG_ERROR "net", "Cannot read: %s", r.Err().no);
			return r.Err().no;
		}
		const char* req = _req.get();

		if (head.command != protocol::status::SUCCESS) {
			protocol::types::error err;
			if (protocol::get_type(req, err)) {
				plog::v(LOG_ERROR "client", "Bad response");
				return -1;
			}

			plog::v(LOG_ERROR "server", "Error: %s", err.msg);
			return -1;
		}

		// read response data
		protocol::types::bigdata fdata;
		if (protocol::get_type(req, fdata)) {
			plog::v(LOG_ERROR "client", "Expected file data");
			return -1;
		}

		if (*(req + fdata.length) != protocol::MAGIC)
			return -1;

		std::stringstream fcontent;
		fcontent.write(req, fdata.length);
		plog::v(LOG_INFO "get",
				"Received %u bytes: \"%s\"",
				fdata.length,
				fcontent.str().c_str());
	}
	return 0;
}

static int put(const std::string& filename, const std::string& filepath) {
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
		if (fstr.fail()) {
			plog::v(LOG_ERROR "fs",
					"Cannot read file %s: %s",
					filepath.c_str(),
					strerror(errno));
			return -1;
		}

		size_t filesize = fstr.tellg();
		fstr.seekg(0, std::ios::beg);

		protocol::types::bigdata fdata(filesize);
		head.length += filesize;

		head.to(tmp);
		fname.to(tmp);
		fdata.to(tmp);

		if (auto r =
				stream.write(tmp.str().c_str(), tmp.str().length(), MSG_MORE)) {
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

			if (auto r = stream.write(blk.get(), bytes_to_write, MSG_MORE)) {
				plog::v(LOG_ERROR "net", "Cannot write: %s", r.Err().msg);
				return r.Err().no;
			}

			written_bytes += 4096;
		}

		// write the magic byte
		if (auto r = stream.write(&protocol::MAGIC, 1, MSG_MORE)) {
			plog::v(LOG_ERROR "net", "Cannot write: %s", r.Err().msg);
			return r.Err().no;
		}

		// Finalize send
		stream.write("", 0);
	}

	{
		// Read the response

		// Read message header
		protocol::types::header head;
		{
			char buf[protocol::types::header::SIZE];
			if (auto r = stream.read(buf, sizeof(buf), MSG_WAITALL)) {
				plog::v(LOG_ERROR "net", "Cannnot read: %s", r.Err().msg);
				return r.Err().no;
			}
			const char* tmp = buf;

			if (protocol::get_type(tmp, head)) {
				plog::v(LOG_ERROR "client", "Bad response");
				return -1;
			}
		}

		std::unique_ptr<char[]> _req(new char[head.length + 1]);
		if (auto r = stream.read(_req.get(), head.length, MSG_WAITALL)) {
			plog::v(LOG_ERROR "net", "Cannot read: %s", r.Err().no);
			return r.Err().no;
		}
		const char* req = _req.get();

		if (head.command != protocol::status::SUCCESS) {
			protocol::types::error err;
			if (protocol::get_type(req, err)) {
				plog::v(LOG_ERROR "client", "Bad response");
				return -1;
			}

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

static int desc(const uid::uid_type& id) {
	{
		std::stringstream tmp;

		protocol::types::header(
			3, protocol::types::string::HEADER_SIZE + id.length())
			.to(tmp);
		protocol::types::string(id.c_str()).to(tmp);

		if (auto r = stream.write(tmp.str().c_str(), tmp.str().length())) {
			plog::v(LOG_ERROR "net", "Cannnot write: %s", r.Err().msg);
			return r.Err().no;
		}
	}

	{
		// Read message header
		protocol::types::header head;
		{
			char buf[protocol::types::header::SIZE];
			if (auto r = stream.read(buf, sizeof(buf), MSG_WAITALL)) {
				plog::v(LOG_ERROR "net", "Cannnot read: %s", r.Err().msg);
				return r.Err().no;
			}
			const char* tmp = buf;

			if (auto err = protocol::get_type(tmp, head)) {
				plog::v(LOG_ERROR "client", "Bad response");
				return -1;
			}
		}

		std::unique_ptr<char[]> _req(new char[head.length + 1]);
		if (auto r = stream.read(_req.get(), head.length, MSG_WAITALL)) {
			plog::v(LOG_ERROR "net", "Cannot read: %s", r.Err().no);
			return r.Err().no;
		}
		const char* req = _req.get();

		if (head.command != protocol::status::SUCCESS) {
			protocol::types::error err;
			if (protocol::get_type(req, err)) {
				plog::v(LOG_ERROR "client", "Bad response");
				return -1;
			}

			plog::v(LOG_ERROR "server", "Error: %s", err.msg);
			return -1;
		}

		// Read the response data
		protocol::types::string fname;
		if (protocol::get_type(req, fname)) {
			plog::v(LOG_ERROR "client", "Expected file name");
			return -1;
		}

		protocol::types::integer fsize;
		if (protocol::get_type(req, fsize)) {
			plog::v(LOG_ERROR "client", "Expected file size");
			return -1;
		}

		plog::v(LOG_NOTICE "client",
				"Name: %s, Size: %u",
				fname.str.c_str(),
				fsize.val);
	}
	return 0;
}

static int del(const uid::uid_type& id) {
	{
		std::stringstream tmp;

		protocol::types::header(
			4, protocol::types::string::HEADER_SIZE + id.length())
			.to(tmp);
		protocol::types::string(id.c_str()).to(tmp);

		if (auto r = stream.write(tmp.str().c_str(), tmp.str().length())) {
			plog::v(LOG_ERROR "net", "Cannnot write: %s", r.Err().msg);
			return r.Err().no;
		}
	}

	{
		// Read message header
		protocol::types::header head;
		{
			char buf[protocol::types::header::SIZE];
			if (auto r = stream.read(buf, sizeof(buf), MSG_WAITALL)) {
				plog::v(LOG_ERROR "net", "Cannnot read: %s", r.Err().msg);
				return r.Err().no;
			}
			const char* tmp = buf;

			if (protocol::get_type(tmp, head)) {
				plog::v(LOG_ERROR "client", "Bad response");
				return -1;
			}
		}

		if (head.command == protocol::status::SUCCESS)
			return 0;

		std::unique_ptr<char[]> _req(new char[head.length + 1]);
		if (auto r = stream.read(_req.get(), head.length, MSG_WAITALL)) {
			plog::v(LOG_ERROR "net", "Cannot read: %s", r.Err().no);
			return r.Err().no;
		}
		const char* req = _req.get();

		protocol::types::error err;
		if (protocol::get_type(req, err)) {
			plog::v(LOG_ERROR "client", "Bad response");
			return -1;
		}

		plog::v(LOG_ERROR "server", "Error: %s", err.msg);
		return -1;
	}
}

static int list(const std::string& filename) {
	{
		// Send the command
		std::stringstream		tmp;
		protocol::types::string fname(filename.c_str());
		protocol::types::header head(
			5, protocol::types::string::HEADER_SIZE + fname.length);

		head.to(tmp);
		fname.to(tmp);

		if (auto r = stream.write(tmp.str().c_str(), tmp.str().length())) {
			plog::v(LOG_ERROR "net", "Cannot write: %s", r.Err().msg);
			return r.Err().no;
		}
	}

	// Read response
	{
		// Read message header
		protocol::types::header head;
		{
			char buf[protocol::types::header::SIZE];
			if (auto r = stream.read(buf, sizeof(buf), MSG_WAITALL)) {
				plog::v(LOG_ERROR "net", "Cannnot read: %s", r.Err().msg);
				return r.Err().no;
			}
			const char* tmp = buf;

			if (auto err = protocol::get_type(tmp, head)) {
				plog::v(LOG_ERROR "client", "Bad response");
				return -1;
			}
		}

		std::unique_ptr<char[]> _req(new char[head.length + 1]);
		if (auto r = stream.read(_req.get(), head.length, MSG_WAITALL)) {
			plog::v(LOG_ERROR "net", "Cannot read: %s", r.Err().no);
			return r.Err().no;
		}
		const char* req = _req.get();

		if (head.command != protocol::status::SUCCESS) {
			protocol::types::error err;
			if (protocol::get_type(req, err)) {
				plog::v(LOG_ERROR "client", "Bad response");
				return -1;
			}

			plog::v(LOG_ERROR "server", "Error: %s", err.msg);
			return -1;
		}

		// read response data
		protocol::types::smallint resultnum;
		if (protocol::get_type(req, resultnum)) {
			plog::v(LOG_ERROR "client", "Expected result number");
			return -1;
		}

		plog::v(LOG_NOTICE "server", "Found %u results:", resultnum.val);
		for (size_t i = 0; i < resultnum.val; i++) {
			protocol::types::string fid;
			if (protocol::get_type(req, fid)) {
				plog::v(LOG_ERROR "client", "Unexpected end of results");
				return -1;
			}

			plog::v(LOG_NOTICE "result", "%s", fid.str.c_str());
		}
	}
	return 0;
}

static int quit() {
	std::stringstream tmp;
	protocol::types::header(6, 0).to(tmp);
	if (auto r = stream.write(tmp.str().c_str(), tmp.str().length())) {
		plog::v(LOG_ERROR "client", "Cannot write: %s", r.Err().msg);
		stream.shutdown();
		return -1;
	}
	stream.shutdown();
	return 0;
}

static void exit_handler(int sig) {
	std::cout << "\n";
	plog::v(LOG_INFO "client", "Exiting...");
	quit();
	exit(sig);
}

int main(int argc, char* argv[]) {
	std::string ip;
	int			port;

	try {
		po::options_description desc("Options");
		desc.add_options()("help,h", "This help message")(
			"ip,i",
			po::value<std::string>(&ip)->default_value("127.0.0.1"),
			"Server address")(
			"port,p",
			po::value<int>(&port)->default_value(DEFAULT_PORT),
			"Port to connect to");

		po::variables_map vm;
		po::store(po::command_line_parser(argc, argv).options(desc).run(), vm);
		po::notify(vm);

		if (vm.count("help")) {
			std::cout << "Usage " << argv[0] << " {flags}\n\n" << desc << "\n";
			return EXIT_SUCCESS;
		}

	} catch (const std::exception& e) {
		plog::v(LOG_ERROR "arg_parser", "%s", e.what());
		return EXIT_FAILURE;
	}

	nio::ip::v4::client cli(nio::ip::v4::addr(ip, port));

	signal(SIGINT, exit_handler);
	signal(SIGTERM, exit_handler);

	if (auto r = cli.Create()) {
		plog::v(LOG_ERROR "net", "Cannot create socket: %s", r.Err().msg);
		exit(r.Err().no);
	}

	if (auto r = cli.Connect()) {
		plog::v(LOG_ERROR "net", "Cannot connect: %s", r.Err().msg);
		exit(r.Err().no);
	} else
		stream = r.Ok();

	// Main shell loop
	std::string prompt =
		stream.peer().ip() + ":" + std::to_string(stream.peer().port()) + " > ";

	std::string input;
	while (true) {
		std::cout << prompt;

		// Ctrl + D
		if (! std::getline(std::cin, input)) {
			exit_handler(0);
		}

		if (input == "")
			continue;

		std::vector<std::string> args;

		std::string		  arg;
		std::stringstream tmp {input};
		while (std::getline(tmp, arg, ' ')) {
			if (arg != "")
				args.push_back(arg);
		}

		std::string command = args[0];
		// shift the args, so args[0] is the first command parameter
		args.erase(args.begin());

		if (command == "GET") {
			if (args.size() != 1)
				plog::v(LOG_NOTICE "Usage", "GET [file id]");
			else
				get(args[0]);
		} else if (command == "PUT") {
			if (args.size() != 2)
				plog::v(LOG_NOTICE "Usage", "PUT [file name] [file path]");
			else
				put(args[0], args[1]);
		} else if (command == "DESC") {
			if (args.size() != 1)
				plog::v(LOG_NOTICE "Usage", "DESC [file id]");
			else
				desc(args[0]);
		} else if (command == "DEL") {
			if (args.size() != 1)
				plog::v(LOG_NOTICE "Usage", "DEL [file id]");
			else
				del(args[0]);
		} else if (command == "LIST") {
			if (args.size() != 1)
				plog::v(LOG_NOTICE "Usage", "LIST [file name]");
			else
				list(args[0]);
		} else if (command == "QUIT") {
			exit_handler(0);
		} else if (command == "HELP") {
			plog::v(LOG_INFO "Commands", "");
			plog::v(
				LOG_NOTICE "GET",
				"\tGet remote file contents from server and print to console");
			plog::v(LOG_NOTICE "PUT", "\tPut local file to server");
			plog::v(LOG_NOTICE "DESC", "\tGet details about remote file");
			plog::v(LOG_NOTICE "DEL", "\tDelete a remote file");
			plog::v(LOG_NOTICE "LIST", "\tList all files with filename");
			plog::v(LOG_NOTICE "QUIT", "\tClose the current connection");
			plog::v(LOG_NOTICE "HELP", "\tThis help message");
		} else {
			plog::v(
				LOG_WARNING "client",
				"Unknown command: \"%s\". Type \"HELP\" for a list of commands",
				command.c_str());
		}
	}

	return 0;
}
