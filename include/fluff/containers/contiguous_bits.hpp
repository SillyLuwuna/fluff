#ifndef FLUFF_CONTAINERS_CONTIGUOUS_BITS_HPP
#define FLUFF_CONTAINERS_CONTIGUOUS_BITS_HPP

#include "fluff/types.hpp"

#include "fluff/containers/bit_array.hpp"

namespace fluff
{
	template <typename T, typename Container, u64 StepSize, u64 Len>
	class ContiguousBits
	{
	private:
		static constexpr bool aligned = (StepSize >> 3 != 0) && ((StepSize & 7) == 0) &&
										(sizeof(Container) * 8 >> 3 != 0) && ((sizeof(Container) * 8 & 7) == 0);

		BitArray<Container, StepSize * Len> bits_;

	public:
		inline constexpr T at(u64 idx) const
		{
			// return bits_.template from_bits<T>(idx * StepSize, StepSize);
			if constexpr (aligned)
			{
				return bits_.template from_bits_fast_aligned<T, StepSize>(idx);
			}
			return bits_.template from_bits_fast<T, StepSize>(idx * StepSize);
		}

		inline constexpr void emplace_at(u64 idx, T&& item)
		{
			// if constexpr (aligned)
			// {
			// 	bits_.template to_bits_fast_aligned<T, StepSize>(idx, std::move(item));
			// }
			bits_.template to_bits_fast<T, false, StepSize>(idx * StepSize, std::move(item));
		}

		inline constexpr void emplace_at(u64 idx, const T& item)
		{
			// if constexpr (aligned)
			// {
			// 	bits_.template to_bits_fast_aligned<T, StepSize>(idx, item);
			// }
			bits_.template to_bits_fast<T, false, StepSize>(idx * StepSize, item);
		}

		inline constexpr void rewrite_at(u64 idx, T&& item)
		{
			// if constexpr (aligned)
			// {
			// 	bits_.template to_bits_fast_aligned<T, StepSize>(idx, std::move(item));
			// }
			bits_.template to_bits_fast<T, true, StepSize>(idx * StepSize, std::move(item));
		}

		inline constexpr void rewrite_at(u64 idx, const T& item)
		{
			// if constexpr (aligned)
			// {
			// 	bits_.template to_bits_fast_aligned<T, StepSize>(idx, item);
			// }
			bits_.template to_bits_fast<T, true, StepSize>(idx * StepSize, item);
		}

		inline constexpr T operator[](u64 idx)
		{
			return at(idx);
		}

		inline constexpr bool cmp(const ContiguousBits<T, Container, StepSize, Len>& other, u64 start, u64 len)
		{
			return bits_.cmp(other.bits_, start * StepSize, len * StepSize);
		}
	};
}


#endif
