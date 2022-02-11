#pragma once

#include <sys/stat.h>

#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#include "common.hpp"
#include "flstream.hpp"
#include "uid.hpp"

namespace storage {

	constexpr auto ID_DIR	= "map/by-id/";
	constexpr auto NAME_DIR = "map/by-name/";
	constexpr auto DATA_DIR = "data/";

	constexpr auto HOOK_DIR		 = "hooks/";
	constexpr auto HOOK_PRE_DIR	 = "hooks/pre.d";
	constexpr auto HOOK_POST_DIR = "hooks/post.d";

	constexpr auto SEPARATOR = '\n';

	struct meta {
		// Last access time (microseconds)
		uint64_t atime;
		// Last modification time (microseconds)
		uint64_t mtime;
		// Size of file
		uint64_t size;

		// Only for use with Result
		meta() {
		}

		meta(const struct stat& _stat);
	};

	class object {
	public:
		uid::uid_type id;
		std::string	  name;
		std::string	  data_path;

		object() {};

		Result<flstream, Error> get(int _mode = flstream::RDONLY);

		Result<meta, Error> details();

		Result<void*, Error> remove();

		static Result<std::vector<uid::uid_type>, Error> by_name(
			const std::string& _name);

		static Result<object, Error> by_id(const uid::uid_type& _id);
	};

	Result<uid::uid_type, Error> create(const std::string& _name);

	/**
	 * @brief Create directory structure @ cwd.
	 *
	 */
	Result<void*, Error> init();

} // namespace storage