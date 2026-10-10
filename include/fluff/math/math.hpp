#ifndef FLUFF_MATH_MATH_HPP
#define FLUFF_MATH_MATH_HPP

#include "fluff/types.hpp"

namespace fluff
{
	struct Math
	{
		static inline constexpr u64 fast_pow(u64 base, u64 pow)
		{
			u64 result = 1;
			while (pow-- != 0)
			{
				result *= base;
			}
			return result;
		}
	};
}


#endif
