#include "ip/v4/stream.hpp"

#include <sys/socket.h>

#include <cerrno>
#include <cstddef>

#include "buffer.hpp"
#include "error.hpp"
#include "ip/v4/addr.hpp"

nio::ip::v4::stream4::stream4(int _sock, const addr4& _p) : _peer(_p) {
	sock = _sock;
}

nio::buffer nio::ip::v4::stream4::read(nio::error& _err,
									   size_t	   _size,
									   int		   _flags) {
	buffer ret(_size);

	int read_bytes = recv(sock, ret.content(), _size, _flags);
	_err		   = errno;

	// dont do unneeded resizing if there was an error
	if (read_bytes >= 0)
		ret.resize(read_bytes);

	return ret;
}

size_t nio::ip::v4::stream4::write(nio::error&	 _err,
								   const buffer& _data,
								   int			 _flags) {
	int written_bytes = send(sock, _data.content(), _data.length(), _flags);
	_err			  = errno;
	return written_bytes;
}

const nio::ip::v4::addr4& nio::ip::v4::stream4::peer() const {
	return _peer;
}