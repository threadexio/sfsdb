#include "storage.hpp"

namespace storage {

	meta::meta(const struct stat& _stat) {
		size  = _stat.st_size;
		atime = _stat.st_atim.tv_nsec / 1000 + _stat.st_atim.tv_sec * 1000000;
		mtime = _stat.st_mtim.tv_nsec / 1000 + _stat.st_mtim.tv_sec * 1000000;
	}

	Result<flstream, Error> object::get(int _mode) {
		Result<flstream, Error> ret;

		flstream data_file(DATA_DIR + id, _mode);

		if (data_file.fail())
			return std::move(ret.Err(std::move(data_file.error().no)));

		return std::move(ret.Ok(std::move(data_file)));
	}

	Result<meta, Error> object::details() {
		Result<meta, Error> ret;

		struct stat stbuf;
		if (stat((DATA_DIR + id).c_str(), &stbuf) < 0)
			return std::move(ret.Err(errno));

		return std::move(ret.Ok(stbuf));
	}

	Result<void*, Error> object::remove() {
		Result<void*, Error> ret;

		try {
			std::filesystem::remove(ID_DIR + id);
			std::filesystem::remove(DATA_DIR + id);
		} catch (const std::filesystem::filesystem_error& e) {
			Error err(e.code().value(), e.what());
			return std::move(ret.Err(std::move(err)));
		}

		flstream name_file(NAME_DIR + name, flstream::RDWR);

		if (name_file.fail())
			return std::move(ret.Err(name_file.error().no));

		name_file.lock(flstream::ltype::WRITE);

		std::string				   chunk;
		std::vector<uid::uid_type> ids;
		while (name_file.getline(chunk, SEPARATOR)) ids.emplace_back(chunk);

		name_file.seek(flstream::SEEK::BEGIN);
		ftruncate(name_file.native_handle(), 0);

		bool empty = true;
		for (auto& fid : ids) {
			if (fid != id) {
				name_file << fid << SEPARATOR;
				empty = false;
			}
		}

		if (empty) {
			std::filesystem::remove(NAME_DIR + name);
			name_file.close();
		}

		return std::move(ret.Ok(nullptr));
	}

	Result<std::vector<uid::uid_type>, Error> object::by_name(
		const std::string& _name) {
		Result<std::vector<uid::uid_type>, Error> ret;

		flstream name_file(NAME_DIR + _name, flstream::RDONLY);

		if (name_file.fail())
			return std::move(ret.Err(name_file.error().no));

		name_file.lock(flstream::ltype::READ);

		std::string				   id;
		std::vector<uid::uid_type> ids;
		while (name_file.getline(id, SEPARATOR)) ids.emplace_back(id);

		return std::move(ret.Ok(std::move(ids)));
	}

	Result<object, Error> object::by_id(const uid::uid_type& _id) {
		Result<object, Error> ret;

		flstream id_file(ID_DIR + _id, flstream::RDONLY);

		if (id_file.fail())
			return std::move(ret.Err(id_file.error().no));

		id_file.lock(flstream::ltype::READ);

		uid::uid_type name;
		id_file.getline(name, SEPARATOR);

		object obj;
		obj.id		  = _id;
		obj.name	  = name;
		obj.data_path = DATA_DIR + _id;

		return std::move(ret.Ok(std::move(obj)));
	}

	Result<uid::uid_type, Error> create(const std::string& _name) {
		Result<uid::uid_type, Error> ret;

		uid::uid_type id = uid::generator().get();

		flstream name_file(
			NAME_DIR + _name,
			flstream::WRONLY | flstream::APPEND | flstream::CREAT);

		if (name_file.fail())
			return std::move(ret.Err(name_file.error().no));

		name_file.lock(flstream::ltype::WRITE);

		flstream id_file(ID_DIR + id,
						 flstream::WRONLY | flstream::CREAT | flstream::TRUNC);

		if (id_file.fail())
			return std::move(ret.Err(std::move(id_file.error().no)));

		id_file.lock(flstream::ltype::WRITE);

		name_file << id << SEPARATOR;

		name_file.close();

		id_file << _name << SEPARATOR;

		flstream data_file(
			DATA_DIR + id,
			flstream::WRONLY | flstream::TRUNC | flstream::CREAT);

		return std::move(ret.Ok(std::move(id)));
	}

	Result<void*, Error> init() {
		Result<void*, Error> ret;
		try {
			std::filesystem::create_directory(DATA_DIR);
			std::filesystem::create_directories(ID_DIR);
			std::filesystem::create_directories(NAME_DIR);

			std::filesystem::create_directories(HOOK_DIR);
			std::filesystem::create_directories(HOOK_PRE_DIR);
			std::filesystem::create_directories(HOOK_POST_DIR);
			return std::move(ret.Ok(nullptr));
		} catch (const std::filesystem::filesystem_error& e) {
			return std::move(ret.Err(e.code().value()));
		}
	}

}; // namespace storage