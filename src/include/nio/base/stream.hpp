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
			/**
			 * @brief Read from the stream _size bytes.
			 *
			 * @param _err Check this for any errors
			 * @param _size Number of bytes to read
			 * @param _flags Special recv() flags. See `man 3 recv`
			 * @return buffer - The bytes read
			 */
			buffer read(error& _err, size_t _size, int _flags = 0) {
				buffer ret(_size);

				int read_bytes = recv(sock, ret.data(), _size, _flags);
				_err		   = errno;

				// dont do unneeded resizing if there was an error
				if (read_bytes >= 0)
					ret.resize(read_bytes);

				return ret;
			}

			/**
			 * @brief Write _data to the stream.
			 *
			 * @param _err Check this for any errors
			 * @param _data The data to write
			 * @param _flags Special send() flags. See `man 3 send`
			 * @return size_t Number of bytes written to the stream
			 */
			size_t write(error& _err, const buffer& _data, int _flags = 0) {
				int written_bytes =
					send(sock, _data.data(), _data.length(), _flags);
				_err = errno;
				return written_bytes;
			}

			/**
			 * @brief Get the address of the remote peer.
			 *
			 * @return const T&
			 */
			const T& peer() const {
				return _peer;
			};

		protected:
			T _peer;
		};
	} // namespace base
} // namespace nio