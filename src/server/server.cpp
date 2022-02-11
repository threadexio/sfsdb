#include "nio/ip/v4/server.hpp"

#include <sched.h>
#include <sys/wait.h>

#include <boost/program_options.hpp>
#include <chrono>
#include <csignal>
#include <string>
#include <thread>

#include "handlers.hpp"
#include "hooks.hpp"
#include "log.hpp"
#include "misc.hpp"
#include "protocol.hpp"
#include "volume.hpp"

namespace po = boost::program_options;

static int con_handler(nio::ip::stream&& _clstream) {
	// One client must be able to send multiple commands per
	// connection
	auto clstream = _clstream;

	for (;;) {
		// Read the message header
		protocol::types::header head;
		{
			char tmp[protocol::types::header::SIZE];
			if (auto r = clstream.read(tmp, sizeof(tmp), MSG_WAITALL))
				break;
			const char* tmp1 = tmp;

			if (auto err = protocol::get_type(tmp1, head)) {
				plog::v(LOG_WARNING "client", "Bad request: %s", err.msg);
				std::stringstream tmp;
				protocol::messages::error(err.msg).to(tmp);
				clstream.write(tmp.str().c_str(), tmp.str().length());
				break;
			}
		}

		std::unique_ptr<char[]> req(new char[head.length]);
		if (auto r = clstream.read(req.get(), head.length, MSG_WAITALL))
			break;

		std::stringstream res;

		auto result =
			protocol::parse(commands, head, req.get(), res, (void*)&clstream);

		// The handler is expected to return HANDLER_ERROR and set
		// the desired response in res if there is an error and the
		// response isn't sent from the handler
		if (result != HANDLER_NO_SEND_RES) {
			if (auto r = clstream.write(res.str().c_str(), res.str().length()))
				break;
		}
	}

	return 0;
}

volume::volume_type vol;

static void child_exit_handler(int) {
	plog::v(LOG_INFO "net", "Closing connection");
	exit(1);
}

static void exit_handler(int sig) {
	// Parse /proc/$$/task/$$/children
	std::string proc_path = "/proc";
	proc_path += getpid();
	proc_path += "/task/";
	proc_path += getpid();
	proc_path += "/children";

	std::fstream chfile(proc_path, std::ios::in);

	std::string		   chpid;
	std::vector<pid_t> children;
	while (std::getline(chfile, chpid, ' '))
		children.push_back(std::stoi(chpid));

	for (auto pid : children) kill(pid, SIGTERM);

	for (auto pid : children) waitpid(pid, NULL, 0);

	if (auto r = hooks::run_hooks(hooks::type::POST))
		plog::v(LOG_ERROR "hook", "%s", r.Err().msg);

	exit(sig);
}

int main(int argc, char* argv[]) {
	std::string volume_path;
	std::string ip;
	int			port;

	try {
		po::options_description desc("Options");
		desc.add_options()("help,h", "This help message")(
			"ip,i",
			po::value<std::string>(&ip)->default_value(DEFAULT_ADDR),
			"Interface address to listen on")(
			"port,p",
			po::value<int>(&port)->default_value(DEFAULT_PORT),
			"Port to listen on")(
			"volume,v",
			po::value<std::string>(&volume_path)->default_value("./"),
			"Path to the volume")("version,V", "Print version");

		po::variables_map vm;
		po::store(po::command_line_parser(argc, argv).options(desc).run(), vm);
		po::notify(vm);

		if (vm.count("version")) {
			plog::v(LOG_INFO "version", VERSION);
			return EXIT_SUCCESS;
		}

		if (vm.count("help")) {
			std::cout << "Usage " << argv[0] << " {flags}\n\n" << desc << "\n";
			return EXIT_SUCCESS;
		}

	} catch (const std::exception& e) {
		plog::v(LOG_ERROR "arg_parser", "%s", e.what());
		return EXIT_FAILURE;
	}

	// Register signal handlers for graceful exits
	signal(SIGINT, exit_handler);
	signal(SIGTERM, exit_handler);

	// Setup the volume
	if (auto r = volume::init(volume_path)) {
		plog::v(LOG_ERROR "volume", "Init: %s", r.Err().msg);
		exit(r.Err().no);
	} else
		vol = r.Ok();

	// Setup network stuff
	nio::ip::v4::addr	addr(ip, port);
	nio::ip::v4::server srv(addr);

	if (auto r = srv.Create()) {
		plog::v(LOG_ERROR "net", "socket: %s", r.Err().msg);
		exit(r.Err().no);
	}

	if (auto r = srv.set_opt(nio::SOPT::REUSE_ADDRESS, &nio::ENABLE)) {
		plog::v(LOG_ERROR "net", "set_opt: %s", r.Err().msg);
		exit(r.Err().no);
	}

	if (auto r = srv.Bind()) {
		plog::v(LOG_ERROR "net", "bind: %s", r.Err().msg);
		exit(r.Err().no);
	}

	if (auto r = srv.Listen()) {
		plog::v(LOG_ERROR "net", "listen: %s", r.Err().msg);
		exit(r.Err().no);
	}

	plog::v(LOG_INFO "net",
			"Listening on %s:%d...",
			addr.ip().c_str(),
			addr.port());

	// Main loop
	for (;;) {
		nio::ip::v4::stream s;
		if (auto r = srv.Accept()) {
			plog::v(LOG_WARNING "net", "accept: %s", r.Err().msg);
			return -1;
		} else
			s = r.Ok();

		plog::v(LOG_INFO "net",
				"Connected: %s:%zu",
				s.peer().ip().c_str(),
				s.peer().port());

		pid_t child = fork();
		if (child == -1) {
			plog::v(LOG_ERROR "sys", "fork: %s", Error(errno).msg);
			continue;
		} else if (child == 0) {
			signal(SIGINT, SIG_DFL);
			signal(SIGTERM, child_exit_handler);
			return con_handler(std::move(s));
		}
	}
	return 0;
}