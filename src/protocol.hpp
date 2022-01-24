#pragma once

#include <cstring>
#include <sstream>
#include <string>
#include <vector>

#include "endian.hpp"

namespace protocol {

	using cmd_type	= uint16_t;
	using size_type = uint32_t;

	using smallint_type = uint16_t;
	using integer_type	= uint32_t;

	/**
	 * @brief Use this to check the type of the next header. Results can be
	 * compared to types::ids or your own types.
	 *
	 * @param in
	 * @return cmd_type
	 */
	inline cmd_type get_type(const char* in) {
		return endian::net::to_hosts(*(cmd_type*)in);
	}

	namespace status {
		constexpr cmd_type SUCCESS = 0;
		constexpr cmd_type ERROR   = 1;
	} // namespace status

	namespace types {

		namespace ids {
			constexpr cmd_type INVALID	= 0;
			constexpr cmd_type HEADER	= 1;
			constexpr cmd_type ERROR	= 2;
			constexpr cmd_type SMALLINT = 3;
			constexpr cmd_type INTEGER	= 4;
			constexpr cmd_type STRING	= 5;
			constexpr cmd_type BIGDATA	= 6;
		} // namespace ids

		template <cmd_type _Type>
		struct data {
		public:
			cmd_type  type	 = _Type;
			size_type length = 0;

			void to(std::stringstream& out) const {
				endian::host::to_nets(out, type);
				endian::host::to_netl(out, length);
				_to(out);
			}

			void from(const char*& in) {
				type   = endian::net::to_hosts(in);
				length = endian::net::to_hostl(in);
				_from(in);
			}

			// The sum of bytes of the 2 fields in data, this is only so we know
			// how much to read.
			static constexpr size_t DATA_HEADER_SIZE =
				sizeof(type) + sizeof(length);

		protected:
			virtual void _to(std::stringstream& out) const = 0;

			virtual void _from(const char*& in) = 0;
		};

		/**
		 * @brief Command header, does not encode any data.
		 *
		 */
		struct header : public data<ids::HEADER> {
			cmd_type command;

			header() {
			}

			header(cmd_type _command, size_type _length) {
				command = _command;
				length	= _length;
			}

			static constexpr size_t SIZE = DATA_HEADER_SIZE + sizeof(command);

		private:
			void _to(std::stringstream& out) const {
				endian::host::to_nets(out, command);
			}

			void _from(const char*& in) {
				command = endian::net::to_hosts(in);
			}
		};

		/**
		 * @brief Fixed-length string used for error messages.
		 *
		 */
		struct error : public data<ids::ERROR> {
			char msg[256];

			error() {
			}

			error(const char* _msg) {
				length = strnlen(_msg, sizeof(msg) - 1);
				strncpy(msg, _msg, length);
				msg[sizeof(msg) - 1] = 0;
			}

		private:
			void _to(std::stringstream& out) const {
				out << msg;
			}

			void _from(const char*& in) {
				strncpy(msg, in, sizeof(msg) - 1);
				msg[sizeof(msg) - 1] = 0;
				in += length;
			}
		};

		/**
		 * @brief Small integer type (uint16).
		 *
		 */
		struct smallint : public data<ids::SMALLINT> {
			smallint_type val;

			smallint() {
			}

			smallint(smallint_type _val) : val(_val) {
				length = sizeof(val);
			}

		private:
			void _to(std::stringstream& out) const {
				endian::host::to_nets(out, val);
			}

			void _from(const char*& in) {
				val = endian::net::to_hosts(in);
			}
		};

		/**
		 * @brief Integer type (uint32).
		 *
		 */
		struct integer : public data<ids::INTEGER> {
			integer_type val;

			integer() {
			}

			integer(integer_type _val) : val(_val) {
				length = sizeof(val);
			}

		private:
			void _to(std::stringstream& out) const {
				endian::host::to_netl(out, val);
			}

			void _from(const char*& in) {
				val = endian::net::to_hostl(in);
			}
		};

		/**
		 * @brief Variable-length string.
		 *
		 */
		struct string : public data<ids::STRING> {
			std::string str;

			string() {
			}

			string(const char* _str) : str(_str) {
				length = str.length();
			}

		private:
			void _to(std::stringstream& out) const {
				out << str;
			}

			void _from(const char*& in) {
				str = std::string(in, length);
				in += length;
			}
		};

		/**
		 * @brief This object represents only the header of some data, that
		 * reading/writing of the data is left up to the caller.
		 *
		 */
		struct bigdata : public data<ids::BIGDATA> {
			bigdata() {
			}

			bigdata(size_type _length) {
				length = _length;
			}

		private:
			void _to(std::stringstream&) const {
			}

			void _from(const char*&) {
			}
		};
	} // namespace types

	using cb_t = int (*)(const types::header&,
						 const char*,
						 std::stringstream&,
						 void*);

	struct command {
		cmd_type no;
		cb_t	 handler;
	};

	using cmd_table = std::vector<command>;

	namespace commands {
		// Special value to indicate which callback should be ran when the
		// command given is not valid
		constexpr cmd_type INVALID = UINT16_MAX - 1;
	} // namespace commands

	inline int parse(
		const cmd_table&	 commands, // Table with commands
		const types::header& head, // Request header (protocol::types::header)
		const char*			 req,  // Pointer to request buffer
		std::stringstream&	 res,  // Response
		void*				 arg = nullptr) {	   // Custom argument
		cb_t invalid = nullptr;

		for (auto& c : commands) {
			if (c.no == commands::INVALID)
				invalid = c.handler;

			if (c.no == head.command)
				if (c.handler != nullptr)
					return c.handler(head, req, res, arg);
		}

		if (invalid != nullptr)
			return invalid(head, req, res, arg);

		return -1;
	}

} // namespace protocol