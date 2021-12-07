#pragma once

#include "../error.hpp"
#include "socket.hpp"

namespace nio {
	namespace base {

		/**
		 * @brief Base class for any client
		 *
		 * @tparam T Type of the corresponding addr class
		 * @tparam E Type of the corresponding stream class
		 */
		template <class T, class E>
		class client : public _sock {
		public:
			/**
			 * @brief Connect to a remote endpoint.
			 *
			 * @param _err Check this for any errors
			 * @return E
			 */
			virtual E Connect(error& _err) = 0;

		protected:
			T remote;
		};

	} // namespace base
} // namespace nio