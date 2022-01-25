#pragma once

#include <cstdint>
#include <filesystem>
#include <string>

#include "common.hpp"
#include "map.hpp"
#include "storage.hpp"
#include "uid.hpp"

namespace volume {

	struct volume_type {
		std::string path;

		// Only for use with Result
		volume_type() {
		}

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
		inline Result<uid::uid_type, Error> store(const std::string& _name,
												  const char* const	 _data,
												  uint64_t			 _len) {
			Result<uid::uid_type, Error> ret;

			uid::uid_type id = "";
			if (auto r = map::by_name(_name).create())
				return std::move(ret.Err(r.Err()));
			else
				id = r.Ok();

			if (auto r = storage::at(id).save(_data, _len))
				return std::move(ret.Err(r.Err()));

			return std::move(ret.Ok(std::move(id)));
		}

		/**
		 * @brief Remove data by it's id
		 *
		 * @param _id
		 * @return true - Successful removal
		 * @return false - Failure
		 */
		inline Result<void*, Error> remove(const uid::uid_type& _id) {
			Result<void*, Error> ret;

			// Nested error checking, this is the way
			if (auto r = map::by_id(_id))
				return std::move(ret.Err(r.Err()));
			else {
				if (auto r1 = r.Ok().remove(_id))
					return std::move(ret.Err(r1.Err()));
			}

			if (auto r = storage::at(_id).remove())
				return std::move(ret.Err(r.Err()));

			return std::move(ret.Ok(nullptr));
		}

		/**
		 * @brief Get a file by name.
		 *
		 * @param _name
		 * @return map::map_type
		 */
		inline map::map_type get_name(const std::string& _name) {
			return map::by_name(_name);
		}

		/**
		 * @brief Get a file by it's id
		 *
		 * @param _id
		 * @return storage::data_type
		 */
		inline Result<storage::data_type, Error> get_id(
			const uid::uid_type& _id) const {
			Result<storage::data_type, Error> ret;

			if (auto r = map::by_id(_id))
				return std::move(ret.Err(r.Err()));
			else
				return std::move(ret.Ok(storage::at(_id)));
		}

		/**
		 * @brief Get mapping by it's id
		 *
		 * @param _id
		 * @return Result<map::map_type, Error>
		 */
		inline Result<map::map_type, Error> get_mapping(
			const uid::uid_type& _id) const {
			Result<map::map_type, Error> ret;

			if (auto r = map::by_id(_id))
				return std::move(ret.Err(r.Err()));
			else
				return std::move(ret.Ok(r.Ok()));
		}
	};

	/**
	 * @brief Initialize the volume in root and cd there.
	 *
	 */
	inline Result<volume_type, Error> init(const std::string& root) {
		Result<volume_type, Error> ret;

		try {
			std::filesystem::create_directory(root);
			std::filesystem::current_path(root);
		} catch (const std::filesystem::filesystem_error& e) {
			return std::move(ret.Err(e.code().value()));
		}

		if (auto r = map::init())
			return std::move(ret.Err(r.Err()));

		if (auto r = storage::init())
			return std::move(ret.Err(r.Err()));

		return std::move(ret.Ok(volume_type(root)));
	}
} // namespace volume