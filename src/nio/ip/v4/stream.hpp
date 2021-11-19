#pragma once

#include <cstddef>

#include "../../base/stream.hpp"
#include "../../buffer.hpp"
#include "../../error.hpp"
#include "addr.hpp"

namespace nio {
	namespace ip {
		namespace v4 {
			class stream4 : public base::stream {
				public:
				stream4(int _sock, const addr4& _p);

				buffer read(nio::error& _err, size_t _size, int _flags = 0);
				size_t write(nio::error&   _err,
							 const buffer& _data,
							 int		   _flags = 0);

				const addr4& peer() const;

				private:
				addr4 _peer;
			};
		} // namespace v4
	}	  // namespace ip
} // namespace nio