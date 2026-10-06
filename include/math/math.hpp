#pragma once

#include <cstdint>

namespace fluff
{
	struct Math
	{
		static inline constexpr uint64_t fast_pow(uint64_t base, uint64_t pow)
		{
			uint64_t result = 1;
			while (pow-- != 0)
			{
				result *= base;
			}
			return result;
		}
	};
}

