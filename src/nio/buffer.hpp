#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

namespace nio {

	class buffer {
		public:
		buffer(size_t _len);
		buffer(void* _data, size_t _len);

		template <class _T, size_t _L = sizeof(_T)>
		_T read() {
			{
				_T ret = *((_T*)&vec[pos]);
				pos += _L;
				return ret;
			}
		}

		template <class _T, size_t _L = sizeof(_T)>
		size_t write(const _T& _data) {
			if (pos + _L > length()) {
				resize(pos + _L);
			}

			memcpy(&vec[pos], &_data, _L);
			pos += _L;

			// return the index at which the data was written
			return pos - _L;
		}

		uint8_t		at(size_t _index) const;
		size_t		length() const;
		bool		empty() const;
		const void* content() const;

		uint8_t& index(size_t _index);
		void*	 content();
		void	 clear();
		void	 seek(size_t _new_pos);
		void	 resize(size_t _len);

		uint8_t& operator[](size_t _index);

		private:
		size_t				 pos = 0;
		std::vector<uint8_t> vec;
	};

} // namespace nio