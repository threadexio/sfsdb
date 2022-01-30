#pragma once
#include <fcntl.h>
#include <unistd.h>

#include <memory>
#include <sstream>
#include <string>

#include "common.hpp"

/**
 * @brief A file stream implementing advisory file locking.
 *
 */
class flstream {
public:
	static constexpr int APPEND	  = O_APPEND;
	static constexpr int ASYNC	  = O_ASYNC;
	static constexpr int CREAT	  = O_CREAT;
	static constexpr int EXCL	  = O_EXCL;
	static constexpr int NOATIME  = O_NOATIME;
	static constexpr int NONBLOCK = O_NONBLOCK;
	static constexpr int RDONLY	  = O_RDONLY;
	static constexpr int RDWR	  = O_RDWR;
	static constexpr int SYNC	  = O_SYNC;
	static constexpr int TRUNC	  = O_TRUNC;
	static constexpr int WRONLY	  = O_WRONLY;

	enum class SEEK {
		BEGIN = SEEK_SET,
		CUR	  = SEEK_CUR,
		END	  = SEEK_END,
	};

	enum class ltype {
		// Read lock, multiple can exist
		READ = F_RDLCK,
		// Write lock, only one can exist
		WRITE = F_WRLCK,
	};

	flstream() {
	}

	flstream(const char *_filepath, int _mode, int _perm = 0660) {
		fl.l_pid	= getpid();
		fl.l_len	= 0;
		fl.l_start	= 0;
		fl.l_whence = SEEK_SET;
		open(_filepath, _mode, _perm);
	}

	flstream(const std::string &_filepath, int _mode, int _perm = 0660) {
		fl.l_pid	= getpid();
		fl.l_len	= 0;
		fl.l_start	= 0;
		fl.l_whence = SEEK_SET;
		open(_filepath.c_str(), _mode, _perm);
	}

	flstream(const flstream &) = delete;

	flstream(flstream &&other) {
		close();
		fd		 = other.fd;
		fl		 = other.fl;
		err		 = std::move(other.err);
		other.fd = -1;
	}

	virtual ~flstream() {
		close();
	}

	flstream &operator=(flstream &&other) {
		close();
		fd		 = other.fd;
		fl		 = other.fl;
		err		 = std::move(other.err);
		other.fd = -1;
		return *this;
	}

	/**
	 * @brief Open a file @ _filepath.
	 *
	 * @param _filepath
	 * @param _mode
	 */
	void open(const char *_filepath, int _mode, int _perm) {
		if (is_open())
			close();

		if ((fd = ::open(_filepath, _mode, _perm)) < 0)
			err.last();
		else
			err.set(0);
	}

	/**
	 * @brief Close the opened file.
	 *
	 */
	void close() {
		unlock_file();
		if (fail())
			return;

		if (::close(fd) < 0)
			err.last();
		else
			err.set(0);
	}

	/**
	 * @brief Read from the file.
	 *
	 * @param _buf Buffer to store read data
	 * @param _len Length of the data to read
	 * @return auto - Amount of bytes read
	 */
	auto read(char *_buf, size_t _len) {
		auto ret = ::read(fd, _buf, _len);
		if (ret < 0)
			err.last();
		else
			err.set(0);

		return ret;
	}

	/**
	 * @brief Write data to the file.
	 *
	 * @param _buf Buffer to write from
	 * @param _len Length of the data to write
	 * @return auto - Amount of bytes written
	 */
	auto write(const char *_buf, size_t _len) {
		auto ret = ::write(fd, _buf, _len);
		if (ret < 0)
			err.last();
		else
			err.set(0);

		return ret;
	}

	/**
	 * @brief Lock the file.
	 *
	 * @param _lock_type Type of the lock
	 * @see flstream::ltype
	 */
	void lock(ltype _lock_type) {
		fl.l_type = static_cast<int>(_lock_type);
		lock_file();
	}

	/**
	 * @brief Unlock the file.
	 *
	 */
	void unlock() {
		unlock_file();
	}

	/**
	 * @brief Check if the file is open.
	 *
	 * @return true
	 * @return false
	 */
	bool is_open() const {
		return fcntl(fd, F_GETFL) != -1;
	}

	/**
	 * @brief Check if there were any errors from the last operation.
	 *
	 * @return true
	 * @return false
	 */
	bool fail() const {
		return err;
	}

	/**
	 * @brief Check errors from the last operation.
	 *
	 * @return const Error&
	 */
	const Error &error() const {
		return err;
	}

	/**
	 * @brief Get the file descriptor.
	 *
	 * @return auto
	 */
	auto native_handle() {
		return fd;
	}

	/**
	 * @brief Read until _delim.
	 *
	 * @param _buf Buffer where the result will be placed
	 * @param _delim The delimiter
	 * @param _chunk_size
	 * @return bool - Whether the delimiter was in the file
	 */
	bool getline(std::string &_buf,
				 char		  _delim	  = '\n',
				 ssize_t	  _chunk_size = 1024) {
		_buf = "";

		std::unique_ptr<char[]> _chunk(new char[_chunk_size]);
		char					 *chunk = _chunk.get();

		size_t passes = 0;
		while (true) {
			memset(chunk, 0, _chunk_size);

			auto read_bytes = read(chunk, _chunk_size);
			if (fail())
				return false;

			if (read_bytes == 0)
				// Return the data we have read until EOF, if it exists
				return passes != 0;

			// Check if delim is in current chunk
			for (auto i = 0; i < read_bytes; i++) {
				if (chunk[i] == _delim) {
					_buf += std::string(chunk, i);

					auto pos_from_start = lseek(fd, 0, SEEK_CUR);

					lseek(fd, pos_from_start - read_bytes + i + 1, SEEK_SET);
					goto finish;
				}
			}

			// This means we have reached EOF
			if (read_bytes < _chunk_size) {
				_buf += std::string(chunk, read_bytes);
				goto finish;
			}

			_buf += std::string(chunk, _chunk_size);

			passes++;
		}
		return false;

	finish:
		return true;
	}

	/**
	 * @brief Seek to _seek + _pos in file.
	 *
	 * @param _seek
	 * @param _pos Offset from _seek
	 */
	void seek(SEEK _seek, size_t _pos = 0) {
		lseek(fd, _pos, static_cast<int>(_seek));
	}

	flstream &operator<<(const char *val) {
		auto len = strlen(val);
		write(val, len);
		return *this;
	}

	flstream &operator<<(const char val) {
		write(&val, 1);
		return *this;
	}

	flstream &operator<<(const std::string &val) {
		write(val.c_str(), val.length());
		return *this;
	}

private:
	int	  fd = -1;
	flock fl;
	Error err;

	void lock_file() {
		if (fcntl(fd, F_SETLKW, &fl) < 0)
			err.last();
		else
			err.set(0);
	}

	void unlock_file() {
		fl.l_type = F_UNLCK;

		if (fcntl(fd, F_SETLKW, &fl) < 0)
			err.last();
		else
			err.set(0);
	}
};