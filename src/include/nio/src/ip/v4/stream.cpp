#include "ip/v4/stream.hpp"

#include "ip/v4/addr.hpp"

nio::ip::v4::stream4::stream4(int _sock, const addr4& _p) {
	_peer = _p;
	sock  = _sock;
}