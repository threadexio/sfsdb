#include "nio/ip/v4/server.hpp"

#include <sched.h>
#include <sys/wait.h>

#include <chrono>
#include <csignal>
#include <string>
#include <thread>

#include "handlers.hpp"
#include "log.hpp"
#include "misc.hpp"
#include "protocol.hpp"
#include "volume.hpp"

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

			if (protocol::get_type(tmp1, head)) {
				plog::v(LOG_WARNING "client", "Bad request");
				break;
			}
		}

		std::unique_ptr<char[]> req(new char[head.length]);
		if (auto r = clstream.read(req.get(), head.length, MSG_WAITALL))
			break;

		std::stringstream res;

		auto result =
			protocol::parse(commands, head, req.get(), res, (void*)&clstream);

		plog::v(LOG_INFO "parser", "Handler status: %d", result);

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

	for (auto pid : children) kill(pid, SIGUSR1);

	for (auto pid : children) waitpid(pid, NULL, 0);

	exit(sig);
}

int main(int argc, char* argv[]) {
	if (argc < 2) {
		plog::v(LOG_NOTICE "Usage", "%s [volume path]", argv[0]);
		return EXIT_FAILURE;
	}

	// Register signal handlers for graceful exits
	signal(SIGINT, exit_handler);
	signal(SIGTERM, exit_handler);

	// Setup the volume
	if (auto r = volume::init(argv[1])) {
		plog::v(LOG_ERROR "volume", "Initialization: %s", r.Err().msg);
		exit(r.Err().no);
	} else
		vol = r.Ok();

	// Setup network stuff
	nio::ip::v4::addr	addr("127.0.0.1", 8888);
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
			plog::v(LOG_ERROR "net", "fork: %s", strerror(errno));
			continue;
		} else if (child == 0) {
			signal(SIGUSR1, child_exit_handler);
			return con_handler(std::move(s));
		}
	}
	return 0;
}