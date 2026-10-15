#ifndef FLUFF_CONTAINERS_CONTIGUOUS_BITS_HPP
#define FLUFF_CONTAINERS_CONTIGUOUS_BITS_HPP

#include "fluff/types.hpp"

#include "fluff/containers/bit_array.hpp"

namespace fluff
{
	using namespace fluff::types;

	/**
	 * Represents a contiguous array in memory.
	 *
	 * The memory may not be aligned, in order to maximize memory efficiency.
	 *
	 * Example: If you have an uint8_t enum that only has 4 possible values, you
	 * could use a StepSize of 2, since only 2 bits are required to save that enum,
	 * and a uint8_t Container. Then Len should be how many enums you are gonna save
	 * in the array, and T would be the enum type.
	 *
	 * @param T the type of the objects in the array.
	 * @param Container the bucket type of the array.
	 * uint8_t would maximize memory efficiency, while Container == T, or with same number
	 * of bytes, would be equivalent to a normal static array.
	 * @param StepSize the number of bits of each object.
	 * This does not need to be the same as the true number of bits.
	 * @param Len the number of objects in the array.
	 */
	template <typename T, typename Container, u64 StepSize, u64 Len>
	class ContiguousArray
	{
		// TODO [] operator with assignment

	private:
		static constexpr bool aligned = (StepSize >> 3 != 0) && ((StepSize & 7) == 0) &&
										(sizeof(Container) * 8 >> 3 != 0) && ((sizeof(Container) * 8 & 7) == 0);

		BitArray<Container, StepSize * Len> bits_;

	public:

		/**
		 * Returns the object at idx.
		 */
		inline constexpr T at(u64 idx) const
		{
			// return bits_.template from_bits<T>(idx * StepSize, StepSize);
			if constexpr (aligned)
			{
				return bits_.template from_bits_fast_aligned<T, StepSize>(idx);
			}
			return bits_.template from_bits_fast<T, StepSize>(idx * StepSize);
		}

		/**
		 * Emplaces item at idx.
		 *
		 * This requires that idx to never have been used, else
		 * memory corruption might occurr. However it is more performant.
		 */
		inline constexpr void emplace_at(u64 idx, T&& item)
		{
			// if constexpr (aligned)
			// {
			// 	bits_.template to_bits_aligned<T, StepSize>(idx, std::move(item));
			// }
			bits_.template to_bits<T, false, StepSize>(idx * StepSize, std::move(item));
		}

		/**
		 * Emplaces item at idx.
		 *
		 * This requires that idx to never have been used, else
		 * memory corruption might occurr. However it is more performant.
		 */
		inline constexpr void emplace_at(u64 idx, const T& item)
		{
			// if constexpr (aligned)
			// {
			// 	bits_.template to_bits_aligned<T, StepSize>(idx, item);
			// }
			bits_.template to_bits<T, false, StepSize>(idx * StepSize, item);
		}

		/**
		 * Emplaces item at idx.
		 *
		 * Slower than emplace_at() but can always be used.
		 */
		inline constexpr void rewrite_at(u64 idx, T&& item)
		{
			// if constexpr (aligned)
			// {
			// 	bits_.template to_bits_fast_aligned<T, StepSize>(idx, std::move(item));
			// }
			bits_.template to_bits_fast<T, true, StepSize>(idx * StepSize, std::move(item));
		}

		/**
		 * Emplaces item at idx.
		 *
		 * Slower than emplace_at() but can always be used.
		 */
		inline constexpr void rewrite_at(u64 idx, const T& item)
		{
			// if constexpr (aligned)
			// {
			// 	bits_.template to_bits_fast_aligned<T, StepSize>(idx, item);
			// }
			bits_.template to_bits_fast<T, true, StepSize>(idx * StepSize, item);
		}

		/**
		 * Returns the object at idx.
		 */
		inline constexpr T operator[](u64 idx)
		{
			return at(idx);
		}

		/**
		 * Checks if the two segments are equal byte-wise.
		 * @param other the ContiguousArray to compare against.
		 * @param start the index to start at.
		 * @param len the number items to compare.
		 */
		inline constexpr bool cmp(const ContiguousArray<T, Container, StepSize, Len>& other, u64 start, u64 len)
		{
			return bits_.cmp(other.bits_, start * StepSize, len * StepSize);
		}
	};
}


#endif
