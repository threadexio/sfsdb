#pragma once

#include <string>

namespace nio {
	class error {
	public:
		int			err = 0;
		std::string msg;

		error();
		error(int _err);

		/**
		 * @brief The the error represented to _err.
		 *
		 * @param _err
		 */
		void set(int _err);

		void operator=(int _err);

		operator bool();
	};
} // namespace nio