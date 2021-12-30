#include "ip/v4/stream.hpp"

#include "ip/v4/addr.hpp"

nio::ip::v4::stream::stream(int _sock, const addr& _p) {
	_peer = _p;
	sock  = _sock;
}