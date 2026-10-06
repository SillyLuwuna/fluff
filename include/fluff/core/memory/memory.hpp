#pragma once

#include <cstdint>
#include <cstring>
#include <iostream>
#include <span>
#include <string>
#include <type_traits>

namespace fluff
{
	struct Mem
	{
		template <typename T, uint64_t Len>
		static inline constexpr void copy(T* lhs, const T* rhs)
		{
			constexpr uint64_t total_bytes = sizeof(T) * Len;

			if (std::is_constant_evaluated())
			{
				std::copy(rhs, rhs + Len, lhs);
			}
			else
			{
				std::memcpy(lhs, rhs, total_bytes);
			}
		}

		template <typename T, uint64_t NumBytes, uint64_t Len = 1>
		static inline constexpr void fill(T* obj, uint8_t value)
		{
			static_assert(std::is_trivially_copyable_v<T>);
			constexpr uint64_t size = sizeof(T);

			if (std::is_constant_evaluated())
			{
				uint64_t curr_byte = 0;
				for (uint64_t i = 0; (i < Len) && (curr_byte < NumBytes); i++)
				{
					std::array<uint8_t, size> bytes = std::bit_cast<std::array<uint8_t, size>>(obj[i]);
					for (uint64_t j = 0; (j < size) && (curr_byte < NumBytes); j++)
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

		template <typename T, uint64_t Len>
		static inline constexpr std::array<uint8_t, Len * sizeof(T)> get_bytes(T* obj)
		{
			constexpr uint64_t size = sizeof(T);

			std::array<uint8_t, Len * size> arr;
			for (uint64_t i = 0; i < Len; i++)
			{
				std::array<uint8_t, size> curr = std::bit_cast<std::array<uint8_t, size>>(obj[i]);
				std::copy(curr.begin(), curr.end(), arr.begin() + (i * size));
			}
			return arr;
		}

		template <typename T, uint64_t Len>
		static inline constexpr void set_bytes(T* obj, const std::array<uint8_t, Len * sizeof(T)>& bytes)
		{
			constexpr uint64_t size = sizeof(T);

			std::span<const uint8_t, Len * size> byte_span(bytes);
			for (uint64_t i = 0; i < Len; i++)
			{
				std::span<const uint8_t> curr = byte_span.subspan(i * size, size);
				std::array<uint8_t, size> arr;
				std::copy(curr.begin(), curr.end(), arr.begin());
				obj[i] = std::bit_cast<T>(arr);
			}
		}

		// TODO constexpr reinterpret cast
	};
}
