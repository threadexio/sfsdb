#include "nio/ip/v4/client.hpp"

#include "nio/ip/v6/client.hpp"
#include "nio/stream.hpp"
#include "nio/unx/client.hpp"
//

#include <boost/program_options.hpp>
#include <csignal>
#include <fstream>
#include <iostream>
#include <memory>
#include <vector>

#include "log.hpp"
#include "misc.hpp"
#include "nio/stream.hpp"
#include "protocol.hpp"
#include "uid.hpp"

namespace po = boost::program_options;

static nio::stream stream;

static int get(const uid::uid_type& id) {
	try {
		{
			// Send the command
			std::stringstream		tmp;
			protocol::types::string fid(id.c_str());
			protocol::types::header head(
				1, protocol::types::string::HEADER_SIZE + fid.length);

			head.to(tmp);
			fid.to(tmp);

			stream.write(tmp.str().c_str(), tmp.str().length());
		}

		// Read response
		{
			// Read message header
			protocol::types::header head;
			{
				char buf[protocol::types::header::SIZE];
				stream.read(buf, sizeof(buf), MSG_WAITALL);
				const char* tmp = buf;

				if (auto err = protocol::get_type(tmp, head)) {
					plog::v(LOG_ERROR "client", "Bad response");
					return -1;
				}
			}

			std::unique_ptr<char[]> _req(new char[head.length + 1]);
			stream.read(_req.get(), head.length, MSG_WAITALL);
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
	} catch (nio::io_error& e) {
		plog::v(LOG_WARNING "net", "%s: %s", e.which(), e.what());
		return -1;
	}
}

static int put(const std::string& filename, const std::string& filepath) {
	try {
		{
			// Construct the command
			std::stringstream tmp;

			protocol::types::header head(
				2,
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

			stream.write(tmp.str().c_str(), tmp.str().length(), MSG_MORE);

			size_t					written_bytes = 0;
			std::unique_ptr<char[]> blk(new char[4096]);
			while (written_bytes < fdata.length) {
				auto bytes_to_write = (fdata.length - written_bytes > 4096)
										  ? 4096
										  : fdata.length - written_bytes;

				fstr.read(blk.get(), bytes_to_write);

				stream.write(blk.get(), bytes_to_write, MSG_MORE);

				written_bytes += 4096;
			}

			// write the magic byte
			stream.write(&protocol::MAGIC, 1, MSG_MORE);

			// Finalize send
			stream.write("", 0);
		}

		{
			// Read the response

			// Read message header
			protocol::types::header head;
			{
				char buf[protocol::types::header::SIZE];
				stream.read(buf, sizeof(buf), MSG_WAITALL);
				const char* tmp = buf;

				if (protocol::get_type(tmp, head)) {
					plog::v(LOG_ERROR "client", "Bad response");
					return -1;
				}
			}

			std::unique_ptr<char[]> _req(new char[head.length + 1]);
			stream.read(_req.get(), head.length, MSG_WAITALL);
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
	} catch (const nio::io_error& e) {
		plog::v(LOG_WARNING "net", "%s: %s", e.which(), e.what());
		return -1;
	}
}

static int desc(const uid::uid_type& id) {
	try {
		{
			std::stringstream tmp;

			protocol::types::header(
				3, protocol::types::string::HEADER_SIZE + id.length())
				.to(tmp);
			protocol::types::string(id.c_str()).to(tmp);

			stream.write(tmp.str().c_str(), tmp.str().length());
		}

		{
			// Read message header
			protocol::types::header head;
			{
				char buf[protocol::types::header::SIZE];
				stream.read(buf, sizeof(buf), MSG_WAITALL);
				const char* tmp = buf;

				if (auto err = protocol::get_type(tmp, head)) {
					plog::v(LOG_ERROR "client", "Bad response");
					return -1;
				}
			}

			std::unique_ptr<char[]> _req(new char[head.length + 1]);
			stream.read(_req.get(), head.length, MSG_WAITALL);
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
	} catch (const nio::io_error& e) {
		plog::v(LOG_WARNING "net", "%s: %s", e.which(), e.what());
		return -1;
	}
}

static int del(const uid::uid_type& id) {
	try {
		{
			std::stringstream tmp;

			protocol::types::header(
				4, protocol::types::string::HEADER_SIZE + id.length())
				.to(tmp);
			protocol::types::string(id.c_str()).to(tmp);

			stream.write(tmp.str().c_str(), tmp.str().length());
		}

		{
			// Read message header
			protocol::types::header head;
			{
				char buf[protocol::types::header::SIZE];
				stream.read(buf, sizeof(buf), MSG_WAITALL);
				const char* tmp = buf;

				if (protocol::get_type(tmp, head)) {
					plog::v(LOG_ERROR "client", "Bad response");
					return -1;
				}
			}

			if (head.command == protocol::status::SUCCESS)
				return 0;

			std::unique_ptr<char[]> _req(new char[head.length + 1]);
			stream.read(_req.get(), head.length, MSG_WAITALL);
			const char* req = _req.get();

			protocol::types::error err;
			if (protocol::get_type(req, err)) {
				plog::v(LOG_ERROR "client", "Bad response");
				return -1;
			}

			plog::v(LOG_ERROR "server", "Error: %s", err.msg);
			return -1;
		}
	} catch (const nio::io_error& e) {
		plog::v(LOG_WARNING "net", "%s: %s", e.which(), e.what());
		return -1;
	}
}

static int list(const std::string& filename) {
	try {
		{
			// Send the command
			std::stringstream		tmp;
			protocol::types::string fname(filename.c_str());
			protocol::types::header head(
				5, protocol::types::string::HEADER_SIZE + fname.length);

			head.to(tmp);
			fname.to(tmp);

			stream.write(tmp.str().c_str(), tmp.str().length());
		}

		// Read response
		{
			// Read message header
			protocol::types::header head;
			{
				char buf[protocol::types::header::SIZE];
				stream.read(buf, sizeof(buf), MSG_WAITALL);
				const char* tmp = buf;

				if (auto err = protocol::get_type(tmp, head)) {
					plog::v(LOG_ERROR "client", "Bad response");
					return -1;
				}
			}

			std::unique_ptr<char[]> _req(new char[head.length + 1]);
			stream.read(_req.get(), head.length, MSG_WAITALL);
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
	} catch (const nio::io_error& e) {
		plog::v(LOG_WARNING "net", "%s: %s", e.which(), e.what());
		return -1;
	}
}

static int quit() {
	try {
		std::stringstream tmp;
		protocol::types::header(6, 0).to(tmp);
		stream.write(tmp.str().c_str(), tmp.str().length());
	} catch (const nio::io_error& e) {}

	stream.close();
	return 0;
}

static void exit_handler(int sig) {
	std::cout << "\n";
	plog::v(LOG_INFO "client", "Exiting...");
	quit();
	exit(sig);
}

static void shell_loop(const std::string& prompt) {
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
			plog::v(LOG_NOTICE "GET",
					"\tGet remote file contents from server and print to "
					"console");
			plog::v(LOG_NOTICE "PUT", "\tPut local file to server");
			plog::v(LOG_NOTICE "DESC", "\tGet details about remote file");
			plog::v(LOG_NOTICE "DEL", "\tDelete a remote file");
			plog::v(LOG_NOTICE "LIST", "\tList all files with filename");
			plog::v(LOG_NOTICE "QUIT", "\tClose the current connection");
			plog::v(LOG_NOTICE "HELP", "\tThis help message");
		} else {
			plog::v(LOG_WARNING "client",
					"Unknown command: \"%s\". Type \"HELP\" for a list of "
					"commands",
					command.c_str());
		}
	}
}

int main(int argc, char* argv[]) {
	std::string ip4;
	std::string ip6;
	std::string unix_path;
	int			port;

	try {
		po::options_description desc("Options");
		desc.add_options()("help,h", "This help message")(
			"ip4,4",
			po::value<std::string>(&ip4),
			"Interface IPv4 address to connect to")(
			"ip6,6",
			po::value<std::string>(&ip6),
			"Interface IPv6 address to connect to")(
			"unix,U",
			po::value<std::string>(&unix_path),
			"Unix socket to connect to")(
			"port,p",
			po::value<int>(&port)->default_value(DEFAULT_PORT),
			"Port to connect to")("version,V", "Print version");

		po::variables_map vm;
		po::store(po::command_line_parser(argc, argv).options(desc).run(), vm);
		po::notify(vm);

		if (vm.count("help")) {
			std::cout << "Usage " << argv[0] << " {flags}\n\n" << desc << "\n";
			return EXIT_SUCCESS;
		}

		if (vm.count("version")) {
			plog::v(LOG_INFO "version", VERSION);
			return EXIT_SUCCESS;
		}

		signal(SIGINT, exit_handler);
		signal(SIGTERM, exit_handler);

		if (vm.count("ip4")) {
			try {
				nio::ip::v4::addr	addr(ip4, port);
				nio::ip::v4::client cli(addr);

				cli.create();
				stream = cli.connect();

				shell_loop(addr.ip() + ":" + std::to_string(addr.port()) +
						   " > ");

				return EXIT_SUCCESS;

			} catch (const nio::error& e) {
				plog::v(LOG_ERROR "net", "%s: %s", e.which(), e.what());
				return EXIT_FAILURE;
			}
		} else if (vm.count("ip6")) {
			try {
				nio::ip::v6::addr	addr(ip6, port);
				nio::ip::v6::client cli(addr);

				cli.create();
				stream = cli.connect();

				shell_loop(addr.ip() + ":" + std::to_string(addr.port()) +
						   " > ");

				return EXIT_SUCCESS;

			} catch (const nio::error& e) {
				plog::v(LOG_ERROR "net", "%s: %s", e.which(), e.what());
				return EXIT_FAILURE;
			}
		} else if (vm.count("unix")) {
			try {
				nio::unx::addr	 addr(unix_path);
				nio::unx::client cli(addr);

				cli.create();
				stream = cli.connect();

				shell_loop(addr.path() + " > ");

				return EXIT_SUCCESS;

			} catch (const nio::error& e) {
				plog::v(LOG_ERROR "net", "%s: %s", e.which(), e.what());
				return EXIT_FAILURE;
			}
		} else {
			plog::v(LOG_ERROR "client",
					"At least one of -4, -6 or -U is required");
			return EXIT_FAILURE;
		}

	} catch (const po::error& e) {
		plog::v(LOG_ERROR "arg_parser", "%s", e.what());
		return EXIT_FAILURE;
	}
}
