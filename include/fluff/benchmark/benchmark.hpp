#ifndef FLUFF_BENCHMARK_BENCHMARK_HPP
#define FLUFF_BENCHMARK_BENCHMARK_HPP

#include "fluff/types.hpp"

#include "fluff/benchmark/byte_measurable.hpp"


namespace fluff::benchmark
{
	using namespace fluff::types;

	/**
	 * Class for all sorts of benchmarking benchmarking
	 * Note: WIP
	 */
	class Benchmark
	{
	private:
	public:

		/**
		 * Tests the number of bytes a certain object uses.
		 * @param utm the object to measure.
		 * @return the number of bytes used.
		 */
		static u64 test_size(const ByteMeasurable& utm)
		{
			return utm.num_bytes();
		}
	};
}

#endif
