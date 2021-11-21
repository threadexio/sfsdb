#include "ip/v4/client.hpp"

#include <sys/socket.h>

#include <cerrno>

#include "error.hpp"
#include "ip/v4/addr.hpp"
#include "ip/v4/stream.hpp"

nio::ip::v4::client4::client4(nio::error& _err, addr4 _remote) {
	remote = _remote;
	sock   = socket(AF_INET, SOCK_STREAM, 0);
	_err   = errno;
}

nio::ip::v4::stream4 nio::ip::v4::client4::Connect(nio::error& _err) {
	addr4 peer;
	(void)connect(sock, remote, remote.length());
	_err = errno;
	getpeername(sock, peer, &peer.length());

	return stream4(sock, peer);
}