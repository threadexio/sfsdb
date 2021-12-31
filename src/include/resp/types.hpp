#pragma once
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace resp {
	namespace types {

		template <typename T>
		struct base {
			T value = 0;

			virtual size_t serialize(char*& _data) const = 0;
		};

		struct sbase : public base<char*> {
			int64_t length = 0;
		};

		struct integer : public base<int64_t> {
			/**
			 * @brief Construct from int64_t
			 *
			 * @param _value
			 */
			integer(int64_t _value);

			/**
			 * @brief Construct from a buffer containing the serialized format
			 *
			 * @param _data
			 */
			integer(char*& _data);

			/**
			 * @brief Serialize and write value to _data, advancing the pointer
			 * forward.
			 *
			 * @param _data Pointer to the buffer
			 * @return size_t - Size of serialized data
			 */
			size_t serialize(char*& _data) const;
		};

		struct simstr : public sbase {
			/**
			 * @brief Construct from a string in memory
			 *
			 * @param _data
			 */
			simstr(const char* _data);

			/**
			 * @brief Construct from a buffer containing the serialized format
			 *
			 * @param _data
			 */
			simstr(char*& _data);

			~simstr();

			/**
			 * @brief Serialize and write value to _data, advancing the pointer
			 * forward.
			 *
			 * @param _data Pointer to the buffer
			 * @return size_t - Size of serialized data
			 */
			size_t serialize(char*& _data) const;
		};

		struct bulkstr : public sbase {
			/**
			 * @brief Construct from a string in memory
			 *
			 * @param _data
			 */
			bulkstr(const char* _data);

			/**
			 * @brief Construct from a buffer containing the serialized format
			 *
			 * @param _data
			 */
			bulkstr(char*& _data);

			~bulkstr();

			/**
			 * @brief Serialize and write value to _data, advancing the pointer
			 * forward.
			 *
			 * @param _data Pointer to the buffer
			 * @return size_t - Size of serialized data
			 */
			size_t serialize(char*& _data) const;
		};

		struct error : sbase {
			/**
			 * @brief Construct from a string in memory
			 *
			 * @param _data
			 */
			error(const char* _data);

			/**
			 * @brief Construct from a buffer containing the serialized format
			 *
			 * @param _data
			 */
			error(char*& _data);

			~error();

			/**
			 * @brief Serialize and write value to _data, advancing the pointer
			 * forward.
			 *
			 * @param _data Pointer to the buffer
			 * @return size_t - Size of serialized data
			 */
			size_t serialize(char*& _data) const;
		};
	} // namespace types
} // namespace resp