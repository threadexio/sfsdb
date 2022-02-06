#pragma once

#include <cstdint>
#include <filesystem>
#include <string>

#include "common.hpp"
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
			if (auto r = storage::create(_name))
				return std::move(ret.Err(r.Err()));
			else
				id = r.Ok();

			storage::object obj;
			if (auto r = storage::object::by_id(id))
				return std::move(ret.Err(r.Err()));
			else
				obj = r.Ok();

			flstream stream;
			if (auto r = obj.get(flstream::WRONLY))
				return std::move(ret.Err(r.Err()));
			else
				stream = r.Ok();

			stream.write(_data, _len);

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
			if (auto r = storage::object::by_id(_id))
				return std::move(ret.Err(r.Err()));
			else {
				if (auto r1 = r.Ok().remove())
					return std::move(ret.Err(r1.Err()));
			}

			return std::move(ret.Ok(nullptr));
		}

		/**
		 * @brief Get a file by name.
		 *
		 * @param _name
		 * @return map::map_type
		 */
		inline auto get_name(const std::string& _name) {
			return storage::object::by_name(_name);
		}

		/**
		 * @brief Get a file by it's id
		 *
		 * @param _id
		 * @return storage::data_type
		 */
		inline auto get_id(const uid::uid_type& _id) const {
			Result<storage::object, Error> ret;

			if (auto r = storage::object::by_id(_id))
				return std::move(ret.Err(r.Err()));
			else {
				if (auto r1 = storage::object::by_id(_id))
					return std::move(ret.Err(r1.Err()));
				else
					return std::move(ret.Ok(r1.Ok()));
			}
		}
	}; // namespace volume

	/**
	 * @brief Initialize the volume in root and cd there.
	 *
	 */
	inline Result<volume_type, Error> init(const std::string& root) {
		Result<volume_type, Error> ret;

		try {
			std::filesystem::current_path(root);
		} catch (const std::filesystem::filesystem_error& e) {
			return std::move(ret.Err(e.code().value()));
		}

		if (auto r = storage::init())
			return std::move(ret.Err(r.Err()));

		return std::move(ret.Ok(volume_type(root)));
	}
} // namespace volume