#pragma once

#include <unistd.h>

namespace nio {
	namespace base {
		class _sock {
			public:
			/**
			 * @brief Get the underlying socket file descriptor.
			 */
			int raw() const {
				return sock;
			}

			/**
			 * @brief Cleanup the socket.
			 */
			virtual void shutdown() {
				close(sock);
			}

			protected:
			int sock;
		};
	} // namespace base
} // namespace nio