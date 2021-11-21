#pragma once

#include <sys/socket.h>

#include "../buffer.hpp"
#include "../error.hpp"
#include "socket.hpp"

namespace nio {
	namespace base {

		/**
		 * @brief Base class for any stream
		 *
		 * @tparam T Type of the corresponding addr class
		 */
		template <class T>
		class stream : public _sock {
			public:
			buffer read(error& _err, size_t _size, int _flags = 0) {
				buffer ret(_size);

				int read_bytes = recv(sock, ret.content(), _size, _flags);
				_err		   = errno;

				// dont do unneeded resizing if there was an error
				if (read_bytes >= 0)
					ret.resize(read_bytes);

				return ret;
			}

			size_t write(error& _err, const buffer& _data, int _flags = 0) {
				int written_bytes =
					send(sock, _data.content(), _data.length(), _flags);
				_err = errno;
				return written_bytes;
			}

			const T& peer() const {
				return _peer;
			};

			protected:
			T _peer;
		};
	} // namespace base
} // namespace nio