#include "map.hpp"

#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

#include "common.hpp"
#include "flstream.hpp"
#include "uid.hpp"

namespace map {
	map_type::map_type(const std::string& _name) {
		name = _name;

		flstream name_file((NAME_DIR + name).c_str(), flstream::RDONLY);

		if (name_file.fail())
			return;

		name_file.lock(flstream::ltype::READ);

		// Parse the file into the vector "ids"
		std::string chunk;
		// The format is <UID><separator>
		// while (std::getline(name_file, chunk, SEPARATOR))
		while (name_file.getline(chunk, SEPARATOR)) ids.emplace_back(chunk);
	}

	Result<uid::uid_type, Error> map_type::create() {
		Result<uid::uid_type, Error> ret;

		flstream name_file((NAME_DIR + name).c_str(),
						   flstream::WRONLY | flstream::CREAT);

		if (name_file.fail())
			return std::move(ret.Err(errno));

		name_file.lock(flstream::ltype::WRITE);

		auto id = uid::generator().get();

		// If the mapping existed before we opened/created it, it means there
		// was at least one uid in there, so write the separator first
		std::string out;
		out.reserve(CHUNK_LEN);

		out += id;
		out += SEPARATOR;

		name_file.write(out.c_str(), out.length());
		name_file.close();

		flstream id_file((ID_DIR + id).c_str(),
						 flstream::WRONLY | flstream::CREAT | flstream::TRUNC);

		if (name_file.fail())
			return std::move(ret.Err(errno));

		id_file.lock(flstream::ltype::WRITE);

		id_file.write((name + SEPARATOR).c_str(),
					  name.length() + SEPARATOR_LEN);

		// Add the id to our vector so it is synced up with the changes
		ids.emplace_back(id);

		return std::move(ret.Ok(std::move(id)));
	}

	Result<void*, Error> map_type::remove(const uid::uid_type& mid) {
		Result<void*, Error> ret;

		if (! exists(mid))
			return std::move(
				ret.Err(ENOENT)); // No such file or directory, interpret this
								  // as "mid does not exist"

		flstream name_file((NAME_DIR + name).c_str(), flstream::RDWR);

		if (name_file.fail())
			return std::move(ret.Err(errno));

		name_file.lock(flstream::ltype::READ);

		// Determines whether the file will be empty after we delete the given
		// uid. We only set it to false if we find data that has to be written
		// again
		bool empty = true;

		std::string buf;
		std::string chunk;
		// while (std::getline(name_file, chunk, SEPARATOR)) {
		while (name_file.getline(chunk, SEPARATOR)) {
			if (chunk == mid)
				continue;

			empty = false;
			buf += chunk;
			buf += SEPARATOR;
		}

		name_file.close();

		if (! empty) { // If we don't have data to write to it, just delete it
			name_file = flstream((NAME_DIR + name).c_str(),
								 flstream::WRONLY | flstream::TRUNC);
			name_file.lock(flstream::ltype::WRITE);
			name_file.write(buf.c_str(), buf.length());
		} else {
			std::filesystem::remove(NAME_DIR + name);
		}

		// Remove the id file
		std::filesystem::remove(ID_DIR + mid);

		// Delete the id from the vector to sync it in order to reflect the
		// changes in the filesystem without reopening and reparsing the file
		for (size_t i = 0; i < ids.size(); i++)
			if (ids[i] == mid)
				ids.erase(ids.begin() + i);

		return std::move(ret.Ok(nullptr));
	}

	Result<map_type, Error> by_id(const uid::uid_type& _id) {
		Result<map_type, Error> ret;

		std::string _name;
		flstream	id_file((ID_DIR + _id).c_str(), flstream::RDONLY);

		if (id_file.fail())
			return std::move(ret.Err(id_file.error().no));

		id_file.lock(flstream::ltype::READ);

		id_file.getline(_name, SEPARATOR);

		return std::move(ret.Ok(std::move(_name)));
	}

	Result<void*, Error> init() {
		Result<void*, Error> ret;
		try {
			std::filesystem::create_directories(ID_DIR);
			std::filesystem::create_directories(NAME_DIR);
			return std::move(ret.Ok(nullptr));
		} catch (const std::filesystem::filesystem_error& e) {
			return std::move(ret.Err(e.code().value()));
		}
	}

}; // namespace map