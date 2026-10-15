#ifndef FLUFF_BENCHMARK_BYTE_MEASURABLE_HPP
#define FLUFF_BENCHMARK_BYTE_MEASURABLE_HPP

#include "fluff/types.hpp"

namespace fluff::benchmark
{
	using namespace fluff::types;

	/**
	 * Interface denoting that the inheriting object's memory usage
	 * can be measured.
	 */
	class ByteMeasurable
	{
	public:
		virtual ~ByteMeasurable() = default;

		/**
		 * The number of bytes the object uses.
		 */
		virtual u64 num_bytes() const = 0;
	};
}

#endif
