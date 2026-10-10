#ifndef FLUFF_RANDOM_RANDOM_ENGINE_HPP
#define FLUFF_RANDOM_RANDOM_ENGINE_HPP

#include "fluff/types.hpp"

namespace fluff::random
{
	class RandomEngine
	{
	private:

	public:
		virtual ~RandomEngine() = default;

		// random 64bit value
		virtual u64 next64() = 0;

		// random 32bit value
		virtual u32 next32() = 0;

		// random 64bit floating point value [0, 1]
		virtual f64 nextf64() = 0;

		// random 32bit floating point value [0, 1]
		virtual f32 nextf32() = 0;
	};
}

#endif
