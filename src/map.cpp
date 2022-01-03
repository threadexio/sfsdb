#include "map.hpp"

#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>
#include <vector>

#include "uid.hpp"

namespace map {
	map_type::map_type(const std::string& _name) {
		name = _name;

		std::fstream name_file(MAP_NAME_DIR + name, std::ios::in);

		if (name_file.fail())
			return;

		// Parse the file into the vector "ids"
		std::string chunk;
		// The format is <UID><separator>
		while (std::getline(name_file, chunk, MAP_SEPARATOR))
			ids.emplace_back(chunk);

		name_file.close();
	}

	uid::uid_type map_type::create() {
		std::fstream name_file(MAP_NAME_DIR + name,
							   std::ios::out | std::ios::app);

		// TODO: Some actual error handling here
		if (name_file.fail())
			return "";

		auto id = uid::generator().get();

		// If the mapping existed before we opened/created it, it means there
		// was at least one uid in there, so write the separator first
		std::string out;
		out.reserve(MAP_CHUNK_LEN);

		out += id;
		out += MAP_SEPARATOR;

		name_file.write(out.c_str(), out.length());
		name_file.close();

		std::fstream id_file(MAP_ID_DIR + id, std::ios::out | std::ios::trunc);
		id_file.write((name + MAP_SEPARATOR).c_str(),
					  name.length() + MAP_SEPARATOR_LEN);
		id_file.close();

		// Add the id to our vector so it is synced up with the changes
		ids.emplace_back(id);

		return id;
	}

	bool map_type::remove(const uid::uid_type& mid) {
		if (! exists(mid))
			return false;

		std::fstream name_file(MAP_NAME_DIR + name,
							   std::ios::in | std::ios::ate);

		// TODO: Some actual error handling here
		if (name_file.fail())
			return false;

		// Simple way to get the size of the file
		size_t map_size = name_file.tellg(); // get current pos, that's the size
		name_file.seekg(0, std::ios::beg);	 // reset pos to the beginning

		// Determines whether the file will be empty after we delete the given
		// uid. We only set it to false if we find data that has to be written
		// again
		bool empty = true;

		std::string buf;
		std::string chunk;
		while (std::getline(name_file, chunk, MAP_SEPARATOR)) {
			if (chunk == mid)
				continue;

			empty = false;
			buf += chunk;
			buf += MAP_SEPARATOR;
		}

		name_file.close();

		if (! empty) { // If we don't have data to write to it, just delete it
			name_file = std::fstream(MAP_NAME_DIR + name,
									 std::ios::out | std::ios::trunc);
			name_file.write(buf.c_str(), buf.length());
			name_file.close();
		} else {
			std::filesystem::remove(MAP_NAME_DIR + name);
		}

		// Remove the id file
		std::filesystem::remove(MAP_ID_DIR + mid);

		// Delete the id from the vector to sync it in order to reflect the
		// changes in the filesystem without reopening and reparsing the file
		for (size_t i = 0; i < ids.size(); i++)
			if (ids[i] == mid)
				ids.erase(ids.begin() + i);

		return true;
	}

	map_type by_id(const uid::uid_type& _id) {
		std::string	 _name;
		std::fstream id_file(MAP_ID_DIR + _id, std::ios::in);
		std::getline(id_file, _name, MAP_SEPARATOR);
		id_file.close();

		return map_type(_name);
	}

	void init() {
		std::filesystem::create_directories(MAP_DIR);
		std::filesystem::create_directory(MAP_ID_DIR);
		std::filesystem::create_directory(MAP_NAME_DIR);
	}

}; // namespace map