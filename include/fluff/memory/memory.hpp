#ifndef FLUFF_MEMORY_MEMORY_HPP
#define FLUFF_MEMORY_MEMORY_HPP

#include "fluff/types.hpp"
#include <cstring>
#include <span>
#include <type_traits>

namespace fluff
{
	struct Mem
	{
		template <typename T, u64 Len>
		static inline constexpr void copy(T* lhs, const T* rhs)
		{
			constexpr u64 total_bytes = sizeof(T) * Len;

			if (std::is_constant_evaluated())
			{
				std::copy(rhs, rhs + Len, lhs);
			}
			else
			{
				std::memcpy(lhs, rhs, total_bytes);
			}
		}

		template <typename T, u64 NumBytes, u64 Len = 1>
		static inline constexpr void fill(T* obj, u8 value)
		{
			static_assert(std::is_trivially_copyable_v<T>);
			constexpr u64 size = sizeof(T);

			if (std::is_constant_evaluated())
			{
				u64 curr_byte = 0;
				for (u64 i = 0; (i < Len) && (curr_byte < NumBytes); i++)
				{
					std::array<u8, size> bytes = std::bit_cast<std::array<u8, size>>(obj[i]);
					for (u64 j = 0; (j < size) && (curr_byte < NumBytes); j++)
					{
						bytes[j] = value;
						curr_byte++;
					}
					obj[i] = std::bit_cast<T>(bytes);
				}
			}
			else
			{
				std::memset(obj, value, NumBytes);
			}
		}

		template <typename T, u64 Len>
		static inline constexpr std::array<u8, Len * sizeof(T)> get_bytes(T* obj)
		{
			constexpr u64 size = sizeof(T);

			std::array<u8, Len * size> arr;
			for (u64 i = 0; i < Len; i++)
			{
				std::array<u8, size> curr = std::bit_cast<std::array<u8, size>>(obj[i]);
				std::copy(curr.begin(), curr.end(), arr.begin() + (i * size));
			}
			return arr;
		}

		template <typename T, u64 Len>
		static inline constexpr void set_bytes(T* obj, const std::array<u8, Len * sizeof(T)>& bytes)
		{
			constexpr u64 size = sizeof(T);

			std::span<const u8, Len * size> byte_span(bytes);
			for (u64 i = 0; i < Len; i++)
			{
				std::span<const u8> curr = byte_span.subspan(i * size, size);
				std::array<u8, size> arr;
				std::copy(curr.begin(), curr.end(), arr.begin());
				obj[i] = std::bit_cast<T>(arr);
			}
		}

		// TODO constexpr reinterpret cast
	};
}

#endif
