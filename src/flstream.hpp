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
  static constexpr int APPEND = O_APPEND;
  static constexpr int ASYNC = O_ASYNC;
  static constexpr int CREAT = O_CREAT;
  static constexpr int EXCL = O_EXCL;
  static constexpr int NOATIME = O_NOATIME;
  static constexpr int NONBLOCK = O_NONBLOCK;
  static constexpr int RDONLY = O_RDONLY;
  static constexpr int RDWR = O_RDWR;
  static constexpr int SYNC = O_SYNC;
  static constexpr int TRUNC = O_TRUNC;
  static constexpr int WRONLY = O_WRONLY;

  enum class ltype {
    // Read lock, multiple can exist
    READ = F_RDLCK,
    // Write lock, only one can exist
    WRITE = F_WRLCK,
  };

  flstream(const char *_filepath, int _mode, int _perm = 0660) {
    fl.l_pid = getpid();
    fl.l_len = 0;
    fl.l_start = 0;
    fl.l_whence = SEEK_SET;
    open(_filepath, _mode, _perm);
  }

  flstream(const flstream &) = delete;

  flstream(flstream &&other) {
    close();
    fd = other.fd;
    fl = other.fl;
    err = std::move(other.err);
    other.fd = -1;
  }

  virtual ~flstream() { close(); }

  flstream &operator=(flstream &&other) {
    close();
    fd = other.fd;
    fl = other.fl;
    err = std::move(other.err);
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
  void unlock() { unlock_file(); }

  /**
   * @brief Check if the file is open.
   *
   * @return true
   * @return false
   */
  bool is_open() const { return fcntl(fd, F_GETFL) != -1; }

  /**
   * @brief Check if there were any errors from the last operation.
   *
   * @return true
   * @return false
   */
  bool fail() const { return err; }

  /**
   * @brief Check errors from the last operation.
   *
   * @return const Error&
   */
  const Error &error() const { return err; }

  /**
   * @brief Get the file descriptor.
   *
   * @return auto
   */
  auto native_handle() { return fd; }

  /**
   * @brief Read until _delim.
   *
   * @param _buf Buffer where the result will be placed
   * @param _delim The delimiter
   * @param _chunk_size
   * @return bool - Whether the delimiter was in the file
   */
  bool getline(std::string &_buf, char _delim = '\n',
               ssize_t _chunk_size = 1024) {
    std::unique_ptr<char[]> b(new char[_chunk_size]);
    char *buf = b.get();

    ssize_t delim_index = 0;
    while (true) {
      memset(buf, 0, _chunk_size);

      auto bytes_read = read(buf, _chunk_size);
      if (fail())
        return false;

      for (ssize_t i = 0; i < _chunk_size; i++) {
        if (buf[i] == _delim) {
          delim_index = i;
          goto exit;
        }
      }
      _buf += std::string(buf, _chunk_size);

      // If we read less bytes than the chunk size, it means we have
      // reached EOF and have not found the delimiter
      if (bytes_read < _chunk_size)
        return false;
    }

  exit:
    _buf += std::string(buf, delim_index);

    return true;
  }

private:
  int fd = -1;
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