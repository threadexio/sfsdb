#include "ip/v4/client.hpp"

#include <sys/socket.h>

#include <cerrno>

#include "error.hpp"
#include "ip/v4/addr.hpp"
#include "ip/v4/stream.hpp"

nio::ip::v4::client::client(nio::error& _err, addr _remote) {
	remote = _remote;
	sock   = socket(AF_INET, SOCK_STREAM, 0);
	_err   = errno;
}

nio::ip::v4::stream nio::ip::v4::client::Connect(nio::error& _err) {
	addr peer;
	(void)connect(sock, remote, remote.length());
	_err = errno;
	getpeername(sock, peer, &peer.length());

	return stream(sock, peer);
}