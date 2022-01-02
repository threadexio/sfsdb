#pragma once
#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>
#include <vector>

#include "uid.hpp"

#define MAP_DIR "map/"

#define MAP_SEPARATOR '\n'

#define MAP_SEPARATOR_LEN 1
#define MAP_CHUNK_LEN	  (UID_TOTAL_LEN + MAP_SEPARATOR_LEN)

static constexpr bool _char_equal(char x, char y) {
	return x == y;
}

static_assert(! _char_equal(MAP_SEPARATOR, UID_SEPARATOR),
			  "MAP and UID separators cannot be the same");

namespace map {

	static inline bool _check_file_exists(const std::string_view& name) {
		return std::filesystem::exists(name);
	}

	struct map_type {
	private:
		std::string _path;

	public:
		std::string				   name;
		std::vector<uid::uid_type> maps;

		map_type(const std::string& _name);

		/**
		 * @brief Check if a mapping exists.
		 *
		 * @param mid
		 * @return true - Mapping exists
		 * @return false - Mapping doesn't exist
		 */
		inline bool exists(const uid::uid_type& mid) {
			for (auto& id : maps)
				if (std::string_view(id.data()) == mid.data())
					return true;
			return false;
		}

		/**
		 * @brief Create a mapping and return the uid that will refer to
		 * it.
		 *
		 * @return uid::uid_type - UID of the new mapping
		 */
		uid::uid_type create();

		/**
		 * @brief Remove a mapping.
		 *
		 * @param name
		 * @param mid Mapping id
		 * @return true - When the mapping was deleted
		 * @return false - When there was an error
		 */
		bool remove(const uid::uid_type& mid);
	};

	/**
	 * @brief Create a mapping for name and return the uid that will refer to
	 * it.
	 *
	 * @param name Name to create the mapping for
	 * @return uid::uid_type - UID of the new mapping
	 */
	uid::uid_type create_mapping(const std::string& name);
}; // namespace map