#pragma once
#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>
#include <vector>

#include "common.hpp"
#include "uid.hpp"

namespace map {

	constexpr const char* ID_DIR		= "map/by-id/";
	constexpr const char* NAME_DIR		= "map/by-name/";
	constexpr const char  SEPARATOR		= '\n';
	constexpr int		  SEPARATOR_LEN = 1;
	constexpr int		  CHUNK_LEN		= (uid::TOTAL_LEN + SEPARATOR_LEN);

	static_assert(SEPARATOR != uid::SEPARATOR,
				  "MAP and UID separators cannot be the same");

	struct map_type {
		std::string				   name;
		std::vector<uid::uid_type> ids;

		map_type() = default;

		map_type(const std::string& _name);

		/**
		 * @brief Create a mapping and return the uid that will refer to
		 * it.
		 *
		 * @return uid::uid_type - ID of the new mapping
		 */
		Result<uid::uid_type, Error> create();

		/**
		 * @brief Remove a mapping.
		 *
		 * @param name
		 * @param mid Mapping id
		 * @return true - When the mapping was successfully deleted
		 * @return false - When there was an error
		 */
		Result<void*, Error> remove(const uid::uid_type& mid);

		/**
		 * @brief Check if a mapping exists.
		 *
		 * @param mid
		 * @return true - Mapping exists
		 * @return false - Mapping doesn't exist
		 */
		inline bool exists(const uid::uid_type& mid) {
			for (auto& id : ids)
				if (id == mid)
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
	Result<map_type, Error> by_id(const uid::uid_type& _id);

	/**
	 * @brief Create the directory structure @ cwd.
	 *
	 */
	Result<void*, Error> init();
}; // namespace map