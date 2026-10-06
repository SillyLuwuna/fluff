#pragma once

#include <cstdint>

namespace fluff
{
	template <uint64_t Num, bool Bit8 = (Num < 256), bool Bit16 = (Num < 65536), bool Bit32 = (Num < 4294967296)>
	struct SizeSelector;

	template <uint64_t num>
	struct SizeSelector<num, true, true, true>
	{
		using type = uint8_t;
	};

	template <uint64_t num>
	struct SizeSelector<num, false, true, true>
	{
		using type = uint16_t;
	};

	template <uint64_t num>
	struct SizeSelector<num, false, false, true>
	{
		using type = uint32_t;
	};

	template <uint64_t num>
	struct SizeSelector<num, false, false, false>
	{
		using type = uint64_t;
	};
}
