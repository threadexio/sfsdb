#pragma once
#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>
#include <vector>

#include "uid.hpp"

#define MAP_DIR		 "map/"
#define MAP_ID_DIR	 MAP_DIR "by-id/"
#define MAP_NAME_DIR MAP_DIR "by-name/"

#define MAP_SEPARATOR '\n'

#define MAP_SEPARATOR_LEN 1
#define MAP_CHUNK_LEN	  (UID_TOTAL_LEN + MAP_SEPARATOR_LEN)

static constexpr bool _char_equal(char x, char y) {
	return x == y;
}

static_assert(! _char_equal(MAP_SEPARATOR, UID_SEPARATOR),
			  "MAP and UID separators cannot be the same");

namespace map {

	struct map_type {
		std::string				   name;
		std::vector<uid::uid_type> ids;

		map_type(const std::string& _name);

		/**
		 * @brief Create a mapping and return the uid that will refer to
		 * it.
		 *
		 * @return uid::uid_type - ID of the new mapping
		 */
		uid::uid_type create();

		/**
		 * @brief Remove a mapping.
		 *
		 * @param name
		 * @param mid Mapping id
		 * @return true - When the mapping was successfully deleted
		 * @return false - When there was an error
		 */
		bool remove(const uid::uid_type& mid);

		/**
		 * @brief Check if a mapping exists.
		 *
		 * @param mid
		 * @return true - Mapping exists
		 * @return false - Mapping doesn't exist
		 */
		inline bool exists(const uid::uid_type& mid) {
			for (auto& id : ids)
				if (std::string_view(id.data()) == mid.data())
					return true;
			return false;
		}
	};

	/**
	 * @brief Retrieve by name.
	 *
	 * @param _name
	 * @return map_type
	 */
	inline map_type by_name(const std::string& _name) {
		return map_type(_name);
	}

	/**
	 * @brief Retrieve by id.
	 *
	 * @param _id
	 * @return map_type
	 */
	map_type by_id(const uid::uid_type& _id);
}; // namespace map