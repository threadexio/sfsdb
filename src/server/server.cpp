#include "nio/ip/v4/server.hpp"

#include "nio/ip/v6/server.hpp"
#include "nio/unx/server.hpp"

//

#include <sched.h>
#include <sys/types.h>
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

static int con_handler(nio::stream&& clstream) {
	// One client must be able to send multiple commands per
	// connection

	for (;;) {
		try {
			// Read the message header
			protocol::types::header head;
			{
				char tmp[protocol::types::header::SIZE];
				clstream.read(tmp, sizeof(tmp), MSG_WAITALL);
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
			clstream.read(req.get(), head.length, MSG_WAITALL);

			std::stringstream res;

			auto result = protocol::parse(
				commands, head, req.get(), res, (void*)&clstream);

			// The handler is expected to return HANDLER_ERROR and set
			// the desired response in res if there is an error and the
			// response isn't sent from the handler
			if (result != HANDLER_NO_SEND_RES) {
				clstream.write(res.str().c_str(), res.str().length());
			}
		}

		catch (const nio::io_error& e) {
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

static void exit_handler(int) {
	// Parse /proc/$$/task/$$/children
	std::string proc_path = "/proc/self/task/";
	proc_path += getpid();
	proc_path += "/children";

	std::fstream	   chfile(proc_path, std::ios::in);
	std::string		   chpid;
	std::vector<pid_t> children;
	while (std::getline(chfile, chpid, ' '))
		children.push_back(std::stoi(chpid));

	for (auto pid : children) kill(pid, SIGTERM);

	for (auto pid : children) waitpid(pid, NULL, 0);

	if (auto r = hooks::run_hooks(hooks::type::POST))
		plog::v(LOG_ERROR "hook", "%s", r.Err().msg);

	exit(0);
}

static int spawn_new_child(nio::stream&& s) {
	pid_t child = fork();
	if (child == -1) {
		plog::v(LOG_ERROR "sys", "fork: %s", Error(errno).msg);
		return -1;
	} else if (child == 0) { /* child code */
		signal(SIGINT, SIG_DFL);
		signal(SIGTERM, child_exit_handler);
		return con_handler(std::move(s));
	} else
		return 1;
}

int main(int argc, char* argv[]) {
	std::string volume_path;
	std::string ip4;
	std::string ip6;
	std::string unix_path;
	int			port;

	try {
		po::options_description desc("Options");
		desc.add_options()("help,h", "This help message")(
			"ip4,4",
			po::value<std::string>(&ip4)->default_value("0.0.0.0"),
			"Interface IPv4 address to listen on")(
			"ip6,6",
			po::value<std::string>(&ip6),
			"Interface IPv6 address to listen on")(
			"unix,U",
			po::value<std::string>(&unix_path),
			"Unix socket to listen on")(
			"port,p",
			po::value<int>(&port)->default_value(DEFAULT_PORT),
			"Port to listen on")("volume,v",
								 po::value<std::string>(&volume_path),
								 "Path to the volume")("version,V",
													   "Print version");

		po::variables_map vm;
		po::store(po::command_line_parser(argc, argv).options(desc).run(), vm);
		po::notify(vm);

		if (vm.count("version")) {
			plog::v(LOG_INFO "sfsdb-server", VERSION);
			return EXIT_SUCCESS;
		}

		if (vm.count("help")) {
			std::cout << "Usage " << argv[0] << " {flags}\n\n" << desc << "\n";
			return EXIT_SUCCESS;
		}

		if (! vm.count("volume")) {
			plog::v(LOG_ERROR "server",
					"Option --volume is required. See --help.");
			return EXIT_FAILURE;
		}

		if (! vm.count("ip4") && ! vm.count("ip6") && ! vm.count("unix")) {
			plog::v(LOG_ERROR "server",
					"At least one of -4, -6 or -U is required");
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

		std::vector<std::thread> subservers;

		// IPv4 section
		if (vm.count("ip4")) {
			subservers.push_back(std::thread([&]() -> int {
				try {
					nio::ip::v4::addr addr(ip4, port);

					nio::ip::v4::server srv(addr);

					srv.create();
					srv.set_opt(nio::option::REUSE_ADDRESS, &nio::ENABLE);
					srv.bind();
					srv.listen();

					plog::v(LOG_NOTICE "net",
							"listening on %s:%d...",
							addr.ip().c_str(),
							addr.port());

					for (;;) {
						try {
							auto stream = srv.accept();

							plog::v(LOG_INFO "net",
									"connected: %s:%d",
									stream.peer().ip().c_str(),
									stream.peer().port());

							if (! spawn_new_child(std::move(stream)))
								continue;
						} catch (const nio::error& e) {
							plog::v(LOG_WARNING "net",
									"%s: %s",
									e.which(),
									e.what());
						}
					}

					return EXIT_SUCCESS;
				} catch (const nio::error& e) {
					plog::v(LOG_ERROR "net", "%s: %s", e.which(), e.what());
					return e.err();
				}
			}));
		}

		// IPv6 section
		if (vm.count("ip6")) {
			subservers.push_back(std::thread([&]() -> int {
				try {
					nio::ip::v6::addr addr(ip6, port);

					nio::ip::v6::server srv(addr);

					srv.create();
					srv.set_opt(nio::option::REUSE_ADDRESS, &nio::ENABLE);
					srv.bind();
					srv.listen();

					plog::v(LOG_NOTICE "net",
							"listening on %s:%d...",
							addr.ip().c_str(),
							addr.port());

					// Main loop
					for (;;) {
						try {
							auto stream = srv.accept();

							plog::v(LOG_INFO "net",
									"connected: %s:%d",
									stream.peer().ip().c_str(),
									stream.peer().port());

							if (! spawn_new_child(std::move(stream)))
								continue;
						} catch (const nio::error& e) {
							plog::v(LOG_WARNING "net",
									"%s: %s",
									e.which(),
									e.what());
						}
					}

					return EXIT_SUCCESS;
				} catch (const nio::error& e) {
					plog::v(LOG_ERROR "net", "%s: %s", e.which(), e.what());
					return e.err();
				}
			}));
		}

		// Unix socket section
		if (vm.count("unix")) {
			subservers.push_back(std::thread([&]() -> int {
				try {
					nio::unx::addr addr;
					addr.path(unix_path);

					nio::unx::server srv(addr);

					plog::v(LOG_NOTICE "net",
							"listening on %s...",
							addr.path().c_str());

					srv.create();
					srv.bind();
					srv.listen();

					for (;;) {
						try {
							if (! spawn_new_child(srv.accept()))
								continue;
						} catch (const nio::error& e) {
							plog::v(LOG_WARNING "net",
									"%s: %s",
									e.which(),
									e.what());
						}
					}

					return EXIT_SUCCESS;
				} catch (const nio::error& e) {
					plog::v(LOG_ERROR "net", "%s: %s", e.which(), e.what());
					return e.err();
				}
			}));
		}

		for (auto& srv : subservers) srv.join();

		if (auto r = hooks::run_hooks(hooks::type::POST))
			plog::v(LOG_ERROR "hook", "%s", r.Err().msg);

		return EXIT_SUCCESS;
	} catch (const po::error& e) {
		plog::v(LOG_ERROR "server", "%s", e.what());
		return EXIT_FAILURE;
	}
}