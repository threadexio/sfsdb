#pragma once

#include <unistd.h>

namespace nio {
	namespace base {
		class _sock {
			public:
			virtual void shutdown() {
				close(sock);
			}

			protected:
			int sock;
		};
	} // namespace base
} // namespace nio