#pragma once

#include <cstring>
#include <sstream>
#include <string>
#include <vector>

#include "common.hpp"
#include "endian.hpp"

namespace protocol {

	constexpr char MAGIC = 0xcc;

	using cmd_type	= uint16_t;
	using size_type = uint32_t;

	using smallint_type = uint16_t;
	using integer_type	= uint32_t;

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

		class data {
		public:
			cmd_type  type	 = ids::INVALID;
			size_type length = 0;

			void to(std::stringstream& out) const {
				endian::host::to_nets(out, type);
				endian::host::to_netl(out, length);
				_to(out);
			}

			/**
			 * @brief Parse the data stream into a usable format
			 *
			 * @param in
			 * @return true - Data was parsed successfully
			 * @return false - Data is invalid
			 */
			bool from(const char*& in) {
				type   = endian::net::to_hosts(in);
				length = endian::net::to_hostl(in);

				switch (type) {
					// We dont check headers
					case ids::HEADER:
						_from(in);
						break;

					// This must be checked manually
					case ids::BIGDATA:
						_from(in);
						break;

					default:
						// Validate length, this is the default behavior
						if (*(in + length) != MAGIC)
							return false;
						_from(in);
						in++;
						break;
				}

				return true;
			}

			// The sum of bytes of the 2 fields in data + 1 magic byte, this is
			// only so we know how much to read.
			static constexpr size_t HEADER_SIZE =
				sizeof(type) + sizeof(length) + 1;

		protected:
			virtual void _to(std::stringstream& out) const = 0;

			virtual void _from(const char*& in) = 0;
		};

		/**
		 * @brief An empty type.
		 *
		 */
		class invalid : public data {
		public:
			invalid() {
				type = ids::INVALID;
			}

			static constexpr size_t SIZE = HEADER_SIZE;

		private:
			void _to(std::stringstream& out) const {
				out << MAGIC;
			}

			void _from(const char*&) {
			}
		};

		/**
		 * @brief Message header, does not encode any data.
		 *
		 */
		class header : public data {
		public:
			cmd_type command;

			header() {
				type = ids::HEADER;
			}

			header(cmd_type _command, size_type _length) {
				type	= ids::HEADER;
				length	= _length;
				command = _command;
			}

			static constexpr size_t SIZE =
				HEADER_SIZE - 1 // headers dont have the magic byte at the end
				+ sizeof(command);

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
		class error : public data {
		public:
			char msg[256];

			error() {
				type = ids::ERROR;
			}

			error(const char* _msg) {
				type   = ids::ERROR;
				length = strnlen(_msg, sizeof(msg) - 1);
				strncpy(msg, _msg, length);
				msg[sizeof(msg) - 1] = 0;
			}

			error& operator=(const char* e) {
				length = strnlen(e, sizeof(msg) - 1);
				strncpy(msg, e, length);
				return *this;
			}

			error& operator=(const std::string& e) {
				length = e.length();
				strncpy(msg, e.c_str(), length);
				return *this;
			}

		private:
			void _to(std::stringstream& out) const {
				out.write(msg, length);
				out << MAGIC;
			}

			void _from(const char*& in) {
				strncpy(msg, in, sizeof(msg) - 1);
				msg[(length > sizeof(msg) - 1) ? sizeof(msg) - 1 : length] = 0;
				in += length;
			}
		};

		/**
		 * @brief Small integer type (uint16).
		 *
		 */
		class smallint : public data {
		public:
			smallint_type val;

			smallint() {
				type = ids::SMALLINT;
			}

			smallint(smallint_type _val) : val(_val) {
				type   = ids::SMALLINT;
				length = sizeof(val);
			}

			smallint& operator=(smallint_type other) {
				val = other;
				return *this;
			}

			static constexpr size_t SIZE = HEADER_SIZE + sizeof(val);

		private:
			void _to(std::stringstream& out) const {
				endian::host::to_nets(out, val);
				out << MAGIC;
			}

			void _from(const char*& in) {
				val = endian::net::to_hosts(in);
			}
		};

		/**
		 * @brief Integer type (uint32).
		 *
		 */
		class integer : public data {
		public:
			integer_type val;

			integer() {
				type = ids::INTEGER;
			}

			integer(integer_type _val) : val(_val) {
				type   = ids::INTEGER;
				length = sizeof(val);
			}

			integer& operator=(integer_type other) {
				val = other;
				return *this;
			}

			static constexpr size_t SIZE = HEADER_SIZE + sizeof(val);

		private:
			void _to(std::stringstream& out) const {
				endian::host::to_netl(out, val);
				out << MAGIC;
			}

			void _from(const char*& in) {
				val = endian::net::to_hostl(in);
			}
		};

		/**
		 * @brief Variable-length string.
		 *
		 */
		class string : public data {
		public:
			std::string str;

			string() {
				type = ids::STRING;
			}

			string(const char* _str) : str(_str) {
				type   = ids::STRING;
				length = str.length();
			}

			string& operator=(const char* e) {
				str	   = e;
				length = str.length();
				return *this;
			}

			string& operator=(const std::string& e) {
				str	   = e;
				length = str.length();
				return *this;
			}

		private:
			void _to(std::stringstream& out) const {
				out << str;
				out << MAGIC;
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
		class bigdata : public data {
		public:
			bigdata() {
				type = ids::BIGDATA;
			}

			bigdata(size_type _length) {
				type   = ids::BIGDATA;
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

	/**
	 * @brief Use this to check the type of the next header.
	 *
	 * @param in Input buffer
	 * @param expected Expected type (types::*)
	 * @return Error - Error.no = 0, if expected type is found. Error.no =
	 * [other type id], if expected type was not found.
	 */
	inline Error get_type(const char*& in, types::data& expected) {
		auto next_type = endian::net::to_hosts(*(cmd_type*)in);
		if (next_type != expected.type)
			return Error(next_type, "Wrong parameter type");

		if (! expected.from(in))
			return Error(-1, "Invalid data format");

		return Error(0);
	}

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