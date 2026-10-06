#pragma once

#include <cstdint>

namespace fluff::random
{
	class RandomEngine
	{
	private:

	public:
		virtual ~RandomEngine() = default;

		// random 64bit value
		virtual uint64_t next64() = 0;

		// random 32bit value
		virtual uint32_t next32() = 0;

		// random 64bit floating point value [0, 1]
		virtual double nextf64() = 0;

		// random 32bit floating point value [0, 1]
		virtual float nextf32() = 0;
	};
}
