#include "ip/v4/server.hpp"

#include <sys/socket.h>

#include "error.hpp"
#include "ip/v4/addr.hpp"
#include "ip/v4/stream.hpp"

nio::ip::v4::server4::server4(nio::error&				_err,
							  const nio::ip::v4::addr4& _server)
	: server(_server) {
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0) {
		_err = sock;
	}
}

nio::error nio::ip::v4::server4::Bind() {
	return bind(sock, server, server.length());
}

nio::error nio::ip::v4::server4::Listen(int _queue) {
	return listen(sock, _queue);
}

nio::ip::v4::stream4 nio::ip::v4::server4::Accept(nio::error& _err) {
	addr4 peer;
	int	  new_stream = accept(sock, peer, &peer.length());
	if (new_stream < 0) {
		_err = new_stream;
	}
	return stream4(new_stream, peer);
}