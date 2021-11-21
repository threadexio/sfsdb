#include "ip/v4/server.hpp"

#include <sys/socket.h>

#include <cerrno>

#include "error.hpp"
#include "ip/v4/addr.hpp"
#include "ip/v4/stream.hpp"

nio::ip::v4::server4::server4(nio::error&				_err,
							  const nio::ip::v4::addr4& _server) {
	server = _server;
	sock   = socket(AF_INET, SOCK_STREAM, 0);
	_err   = errno;
}

nio::error nio::ip::v4::server4::Bind() {
	// we dont actually need the return value for anything
	// set _err to errno to indicate success or an error
	(void)bind(sock, server, server.length());
	return error(errno);
}

nio::error nio::ip::v4::server4::Listen(int _queue) {
	(void)listen(sock, _queue);
	return error(errno);
}

nio::ip::v4::stream4 nio::ip::v4::server4::Accept(nio::error& _err) {
	addr4 peer;
	int	  new_stream = accept(sock, peer, &peer.length());
	_err			 = errno;
	return stream4(new_stream, peer);
}