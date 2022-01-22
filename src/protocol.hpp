#pragma once

#include <sstream>
#include <vector>

namespace protocol {

	using cmd_type	= uint16_t;
	using size_type = uint32_t;

	struct header {
		cmd_type  command;
		size_type length;

		void to(char* out) const;

		void to(std::stringstream& out) const;

		static header from(const char* _data);
	};

	constexpr size_t HEADER_SIZE =
		sizeof(header::command) + sizeof(header::length);

	using cb_t = int (*)(const header&, char*, std::stringstream&, void* arg);

	struct command {
		cmd_type no;
		cb_t	 handler;
	};

	using cmd_table = std::vector<command>;

	constexpr cmd_type SUCCESS_HANDLER = 0;
	constexpr cmd_type INVALID_HANDLER = 1;
	constexpr cmd_type ERROR_HANDLER   = 2;

	int parse(const cmd_table&	 commands,
			  const header&		 head,
			  char*				 req,
			  std::stringstream& res,
			  void*				 arg);

	namespace types {
		struct error {
			header head;
			char   msg[256];

			error();

			error(const char* _msg);

			void to(std::stringstream& out) const;

			static error from(char*& _data);
		};
	} // namespace types

} // namespace protocol