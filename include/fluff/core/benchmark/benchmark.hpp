#pragma once

#include <cstdint>

#include "fluff/core/benchmark/byte_measurable.hpp"

namespace fluff::benchmark
{
	class Benchmark
	{
	private:
	public:

		static uint64_t test_size(const ByteMeasurable& utm)
		{
			return utm.num_bytes();
		}
	};
}
