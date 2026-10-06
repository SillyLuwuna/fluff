#pragma once

#include <cstdint>

namespace fluff::benchmark
{
	class ByteMeasurable
	{
	public:
		virtual ~ByteMeasurable() = default;

		virtual uint64_t num_bytes() const = 0;
	};
}
