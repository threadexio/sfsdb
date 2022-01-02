#include "map.hpp"

#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "uid.hpp"

namespace map {

	map_type::map_type(const std::string& _name) {
		name  = _name;
		_path = MAP_DIR + _name;

		if (! _check_file_exists(_path))
			return;

		std::fstream map_file(_path, std::ios::in);

		if (map_file.fail())
			return;

		// The format is <UID><separator>
		std::string chunk;
		while (std::getline(map_file, chunk, MAP_SEPARATOR))
			maps.emplace_back(chunk);

		map_file.close();
	}

	uid::uid_type map_type::create() {
		return create_mapping(name);
	}

	bool map_type::remove(const uid::uid_type& mid) {
		if (! exists(mid))
			return false;

		std::fstream map_file(_path, std::ios::in | std::ios::ate);

		if (map_file.fail())
			return false;

		size_t map_size = map_file.tellg();
		map_file.seekg(0, std::ios::beg);

		std::string buf;
		std::string chunk;
		while (std::getline(map_file, chunk, MAP_SEPARATOR)) {
			if (chunk == mid)
				continue;

			buf += chunk;
			buf += MAP_SEPARATOR;
		}

		map_file.close();
		map_file = std::fstream(_path, std::ios::out | std::ios::trunc);

		map_file.write(buf.c_str(), buf.length());

		return exists(mid);
	}

	uid::uid_type create_mapping(const std::string& name) {
		std::fstream map_file(MAP_DIR + name, std::ios::out | std::ios::app);

		if (map_file.fail())
			return "";

		auto id = uid::generator().get();

		// If the mapping existed before we opened/created it, it means there
		// was at least one uid in there, so write the separator first
		std::string out;
		out.reserve(MAP_CHUNK_LEN);

		out += id;
		out += MAP_SEPARATOR;

		map_file.write(out.c_str(), out.length());
		map_file.close();

		return id;
	}

}; // namespace map