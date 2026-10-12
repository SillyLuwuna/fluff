#ifndef FLUFF_BENCHMARK_BENCHMARK_HPP
#define FLUFF_BENCHMARK_BENCHMARK_HPP

#include "fluff/types.hpp"

#include "fluff/benchmark/byte_measurable.hpp"


namespace fluff::benchmark
{
	using namespace fluff::types;

	class Benchmark
	{
	private:
	public:

		static u64 test_size(const ByteMeasurable& utm)
		{
			return utm.num_bytes();
		}
	};
}

#endif
