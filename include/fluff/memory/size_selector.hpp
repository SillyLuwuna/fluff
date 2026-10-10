#ifndef FLUFF_MEMORY_SIZE_SELECTOR_HPP
#define FLUFF_MEMORY_SIZE_SELECTOR_HPP

#include "fluff/types.hpp"

namespace fluff
{
	template <u64 Num, bool Bit8 = (Num < 256), bool Bit16 = (Num < 65536), bool Bit32 = (Num < 4294967296)>
	struct SizeSelector;

	template <u64 num>
	struct SizeSelector<num, true, true, true>
	{
		using type = u8;
	};

	template <u64 num>
	struct SizeSelector<num, false, true, true>
	{
		using type = u16;
	};

	template <u64 num>
	struct SizeSelector<num, false, false, true>
	{
		using type = u32;
	};

	template <u64 num>
	struct SizeSelector<num, false, false, false>
	{
		using type = u64;
	};
}

#endif
