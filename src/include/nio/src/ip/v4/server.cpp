#include "ip/v4/server.hpp"

#include <sys/socket.h>

#include <cerrno>

#include "error.hpp"
#include "ip/v4/addr.hpp"
#include "ip/v4/stream.hpp"

nio::ip::v4::server::server(nio::error& _err, const nio::ip::v4::addr& _srv) {
	srv	 = _srv;
	sock = socket(AF_INET, SOCK_STREAM, 0);
	_err = errno;
}

nio::error nio::ip::v4::server::Bind() {
	// we dont actually need the return value for anything
	// set _err to errno to indicate success or an error
	(void)bind(sock, srv, srv.length());
	return error(errno);
}

nio::error nio::ip::v4::server::Listen(int _queue) {
	(void)listen(sock, _queue);
	return error(errno);
}

nio::ip::v4::stream nio::ip::v4::server::Accept(nio::error& _err) {
	addr peer;
	int	 new_stream = accept(sock, peer, &peer.length());
	_err			= errno;
	return stream(new_stream, peer);
}