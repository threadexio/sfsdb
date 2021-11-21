#pragma once

#include "../error.hpp"
#include "socket.hpp"

namespace nio {
	namespace base {

		/**
		 * @brief Base class for any server
		 *
		 * @tparam T Type of the corresponding addr class
		 * @tparam E Type of the corresponding stream class
		 */
		template <class T, class E>
		class server : public _sock {
			public:
			virtual error Bind()				 = 0;
			virtual error Listen(int _queue = 5) = 0;
			virtual E	  Accept(error& _err)	 = 0;

			protected:
			T server;
		};

	} // namespace base
} // namespace nio