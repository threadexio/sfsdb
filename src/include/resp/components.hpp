#pragma once
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace resp {
	namespace components {
		struct integer {
			int64_t value = 0;

			integer(char*& _data);
		};

		struct string {
			char*	value  = NULL;
			int64_t length = 0;

			string(char*& _data);

			~string();
		};

		struct error {
			char*	value  = NULL;
			int64_t length = 0;

			error(char*& _data);

			~error();
		};
	} // namespace components
} // namespace resp