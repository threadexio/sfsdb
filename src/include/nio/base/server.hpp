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
			/**
			 * @brief Bind to the server address.
			 *
			 * @return error - Check this for errors
			 */
			virtual error Bind() = 0;

			/**
			 * @brief Listen on the server address.
			 *
			 * @param _queue
			 * @return error - Check this for errors
			 */
			virtual error Listen(int _queue = 5) = 0;

			/**
			 * @brief Accept a new connection from the
			 *
			 * @param _err Check this for errors
			 * @return E
			 */
			virtual E Accept(error& _err) = 0;

		protected:
			T server;
		};

	} // namespace base
} // namespace nio