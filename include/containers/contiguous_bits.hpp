#pragma once

#include <bitset>
#include <cstdint>

#include "containers/bit_array.hpp"

namespace fluff
{
	template <typename T, typename Container, uint64_t StepSize, uint64_t Len>
	class ContiguousBits
	{
	private:
		static constexpr bool aligned = (StepSize >> 3 != 0) && ((StepSize & 7) == 0) &&
										(sizeof(Container) * 8 >> 3 != 0) && ((sizeof(Container) * 8 & 7) == 0);

		BitArray<Container, StepSize * Len> bits_;

	public:
		inline constexpr T at(uint64_t idx) const
		{
			// return bits_.template from_bits<T>(idx * StepSize, StepSize);
			if constexpr (aligned)
			{
				return bits_.template from_bits_fast_aligned<T, StepSize>(idx);
			}
			return bits_.template from_bits_fast<T, StepSize>(idx * StepSize);
		}

		inline constexpr void emplace_at(uint64_t idx, T&& item)
		{
			// if constexpr (aligned)
			// {
			// 	bits_.template to_bits_fast_aligned<T, StepSize>(idx, std::move(item));
			// }
			bits_.template to_bits_fast<T, false, StepSize>(idx * StepSize, std::move(item));
		}

		inline constexpr void emplace_at(uint64_t idx, const T& item)
		{
			// if constexpr (aligned)
			// {
			// 	bits_.template to_bits_fast_aligned<T, StepSize>(idx, item);
			// }
			bits_.template to_bits_fast<T, false, StepSize>(idx * StepSize, item);
		}

		inline constexpr void rewrite_at(uint64_t idx, T&& item)
		{
			// if constexpr (aligned)
			// {
			// 	bits_.template to_bits_fast_aligned<T, StepSize>(idx, std::move(item));
			// }
			bits_.template to_bits_fast<T, true, StepSize>(idx * StepSize, std::move(item));
		}

		inline constexpr void rewrite_at(uint64_t idx, const T& item)
		{
			// if constexpr (aligned)
			// {
			// 	bits_.template to_bits_fast_aligned<T, StepSize>(idx, item);
			// }
			bits_.template to_bits_fast<T, true, StepSize>(idx * StepSize, item);
		}

		inline constexpr T operator[](uint64_t idx)
		{
			return at(idx);
		}

		inline constexpr bool cmp(const ContiguousBits<T, Container, StepSize, Len>& other, uint64_t start, uint64_t len)
		{
			return bits_.cmp(other.bits_, start * StepSize, len * StepSize);
		}
	};
}

