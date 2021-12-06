#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

namespace nio {

	/**
	 * @brief Generic buffer class to hold a dynamically allocated byte buffer.
	 */
	class buffer {
		public:
		buffer() {};

		/**
		 * @brief Create a new buffer with initial size of _len.
		 *
		 * @param _len
		 */
		buffer(size_t _len);

		/**
		 * @brief Create a new buffer copying data from a memory address.
		 *
		 * @param _data Pointer to the data
		 * @param _len Length of the data
		 */
		buffer(void* _data, size_t _len);

		/**
		 * @brief Read an object from the buffer.
		 *
		 * @tparam _T Type of the object to read
		 * @tparam _L Length of the object in bytes. Defaults to sizeof(_T)
		 * @return _T The object read
		 */
		template <class _T, size_t _L = sizeof(_T)>
		_T read() {
			{
				_T ret = *((_T*)&vec[pos]);
				pos += _L;
				return ret;
			}
		}

		/**
		 * @brief Write an object to the buffer.
		 *
		 * @tparam _T Type of the object
		 * @tparam _L Length of the object in bytes. Defaults to sizeof(_T)
		 * @param _data A const reference to the object
		 * @return size_t The index at which the object was written
		 */
		template <class _T, size_t _L = sizeof(_T)>
		size_t write(const _T& _data) {
			if (pos + _L > length()) {
				resize(pos + _L);
			}

			memcpy(&vec[pos], &_data, _L);
			pos += _L;

			return pos - _L;
		}

		/**
		 * @brief Get a reference of the byte at _index.
		 *
		 * @param _index
		 * @return uint8_t
		 */
		uint8_t& at(size_t _index);

		/**
		 * @brief Get the length of the buffer.
		 *
		 * @return size_t
		 */
		size_t length() const;

		/**
		 * @brief Check whether the buffer is empty or not.
		 *
		 * @return true
		 * @return false
		 */
		bool empty() const;

		/**
		 * @brief Get a read-only pointer to the buffer's data.
		 *
		 * @return const void*
		 */
		const void* data() const;

		/**
		 * @brief Get a read-write pointer to the buffer's data.
		 *
		 * @return void*
		 */
		void* data();

		/**
		 * @brief Clear out the buffer.
		 */
		void clear();

		/**
		 * @brief Set the read/write offset.
		 *
		 * @param _new_pos
		 */
		void seek(size_t _new_pos);

		/**
		 * @brief Resize the buffer to _len.
		 *
		 * @param _len
		 */
		void resize(size_t _len);

		uint8_t& operator[](size_t _index);

		operator char*();

		private:
		size_t				 pos = 0;
		std::vector<uint8_t> vec;
	};

} // namespace nio