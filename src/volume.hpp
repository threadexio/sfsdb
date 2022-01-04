#pragma once

#include <cstdint>
#include <filesystem>
#include <string>

#include "map.hpp"
#include "storage.hpp"
#include "uid.hpp"

namespace volume {

	struct volume_type {
		std::string path;

		volume_type(const std::string& _path) : path(_path) {
		}

		/**
		 * @brief Store a new file in the volume.
		 *
		 * @param _name Filename
		 * @param _data Buffer with data
		 * @param _len Length of data
		 * @return uid::uid_type
		 */
		inline uid::uid_type store(const std::string& _name,
								   const char* const  _data,
								   uint64_t			  _len) {
			auto id = map::by_name(_name).create();
			storage::at(id).save(_data, _len);
			return id;
		}

		/**
		 * @brief Remove data by it's id
		 *
		 * @param _id
		 * @return true - Successful removal
		 * @return false - Failure
		 */
		inline bool remove(const uid::uid_type& _id) {
			map::by_id(_id).remove(_id);
			storage::at(_id).remove();
			return ! std::filesystem::exists(STOR_DATA_DIR + _id);
		}

		/**
		 * @brief Get a list of files with that name
		 *
		 * @param _name
		 * @return map::map_type
		 */
		inline map::map_type get_name(const uid::uid_type& _name) {
			return map::by_name(_name);
		}

		/**
		 * @brief Get a file by it's id
		 *
		 * @param _id
		 * @return storage::data_type
		 */
		inline storage::data_type get_id(const uid::uid_type& _id) {
			// This check is really bad, but it works until some actual error
			// handling is implemented
			if (! map::by_id(_id).exists(_id)) {
				// error handling when?
			}

			return storage::at(_id);
		}
	};

	/**
	 * @brief Initialize the volume in root and cd there.
	 *
	 */
	inline volume_type init(const std::string& root) {
		std::filesystem::create_directory(root);
		std::filesystem::current_path(root);

		map::init();
		storage::init();

		return volume_type(root);
	}
} // namespace volume