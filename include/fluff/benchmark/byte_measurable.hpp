#ifndef FLUFF_BENCHMARK_BYTE_MEASURABLE_HPP
#define FLUFF_BENCHMARK_BYTE_MEASURABLE_HPP

#include "fluff/types.hpp"

namespace fluff::benchmark
{
	class ByteMeasurable
	{
	public:
		virtual ~ByteMeasurable() = default;

		virtual u64 num_bytes() const = 0;
	};
}

#endif
