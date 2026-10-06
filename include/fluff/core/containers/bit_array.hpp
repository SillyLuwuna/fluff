#pragma once

#include "fluff/core/hashing/hasher.hpp"
#include "fluff/core/memory/memory.hpp"
#include <cmath>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

namespace fluff
{
	using namespace fluff;

	template <typename Container, uint64_t NumBits>
	class BitArray
	{
	private:
		template<typename OtherContainer, uint64_t OtherNumBits>
		friend class BitArray;

		static constexpr uint64_t container_bytes_ = sizeof(Container);
		static constexpr uint64_t container_bits_ = container_bytes_ * 8;
		static constexpr uint64_t container_division_ = std::bit_width(container_bits_) - 1; // log2
		static constexpr uint64_t container_modulus_ = container_bits_ - 1;

		static constexpr uint64_t chunk_size_ = ((NumBits - 1) / container_bits_) + 1;
		static constexpr uint64_t total_bytes_ = container_bytes_ * chunk_size_;

		Container bit_chunks_[chunk_size_];

		static inline constexpr uint64_t chunk(uint64_t idx)
		{
			return idx >> container_division_;
		}

		static inline constexpr uint64_t chunk_idx(uint64_t idx)
		{
			return idx & container_modulus_;
		}

		enum Operator : uint8_t
		{
			Or,
			And,
			Xor
		};

		template <typename LhsContainer, typename RhsContainer, Operator Op>
		static inline constexpr void execute_op(LhsContainer* op_lhs, RhsContainer op_rhs)
		{
			if constexpr (Op == Operator::Or) *op_lhs |= op_rhs;
			else if constexpr (Op == Operator::And) *op_lhs &= op_rhs;
			else if constexpr (Op == Operator::Xor) *op_lhs ^= op_rhs;
			else throw std::runtime_error("Unknown bit array operation");
		}

		template <typename GeneralContainer, uint64_t LhsNumBits, uint64_t RhsNumBits, Operator Op>
		static inline constexpr void apply_operator(BitArray<GeneralContainer, LhsNumBits>& lhs, const BitArray<GeneralContainer, RhsNumBits>& rhs)
		{
			constexpr uint64_t min_num_chunks = std::min(lhs.chunk_size_, rhs.chunk_size_);

			for(uint64_t i = 0; i < min_num_chunks; i++)
			{
				execute_op(lhs.bit_chunks_ + i, rhs.bit_chunks_[i]);
			}
		}

		template <typename LhsContainer, uint64_t LhsNumBits, typename RhsContainer, uint64_t RhsNumBits, Operator Op>
		static constexpr void apply_operator(BitArray<LhsContainer, LhsNumBits>& lhs, const BitArray<RhsContainer, RhsNumBits>& rhs)
		{
			using Lhs = BitArray<LhsContainer, LhsNumBits>;
			using Rhs = BitArray<RhsContainer, RhsNumBits>;
			constexpr bool is_lhs_larger = Lhs::container_bytes_ > Rhs::container_bytes_;
			constexpr uint8_t byte_ratio = is_lhs_larger ? (Lhs::container_bytes_ / Rhs::container_bytes_) : (Rhs::container_bytes_ / Lhs::container_bytes_);
			constexpr uint64_t chunks_aligned = is_lhs_larger ? (Rhs::total_bytes_ / Lhs::container_bytes_) : (Lhs::total_bytes_ / Rhs::container_bytes_);
			constexpr uint8_t bytes_missaligned = is_lhs_larger ? (Rhs::total_bytes_ % Lhs::container_bytes_) : (Lhs::total_bytes_ % Rhs::container_bytes_);
			constexpr uint8_t chunks_missaligned = is_lhs_larger ? (bytes_missaligned / Rhs::container_bytes_) : (bytes_missaligned / Lhs::container_bytes_);

			for(uint64_t i = 0; i < chunks_aligned; i++)
			{
				if constexpr (is_lhs_larger)
				{
					LhsContainer* op_lhs = lhs.bit_chunks_ + i;
					LhsContainer op_rhs = *((LhsContainer*)(rhs.bit_chunks_ + (i * byte_ratio)));
					execute_op<LhsContainer, LhsContainer, Op>(op_lhs, op_rhs);
				}
				else
				{
					RhsContainer* op_lhs = (RhsContainer*)(lhs.bit_chunks_ + (i * byte_ratio));
					RhsContainer op_rhs = rhs.bit_chunks_[i];
					execute_op<RhsContainer, RhsContainer, Op>(op_lhs, op_rhs);
				}
			}

			uint64_t curr_minor_chunk = 0;
			for (uint64_t i = chunks_aligned * byte_ratio; i < (chunks_missaligned + (chunks_aligned * byte_ratio)); i++)
			{
				if constexpr (is_lhs_larger)
				{
					LhsContainer* op_lhs = lhs.bit_chunks_ + chunks_aligned;
					LhsContainer op_rhs = ((LhsContainer)rhs.bit_chunks_[i]) << ((curr_minor_chunk++) * rhs.container_bits_);
					execute_op<LhsContainer, LhsContainer, Op>(op_lhs, op_rhs);
				}
				else
				{
					LhsContainer* op_lhs = lhs.bit_chunks_ + i;
					LhsContainer op_rhs = (LhsContainer)((rhs.bit_chunks_[chunks_aligned]) >> ((curr_minor_chunk++) * lhs.container_bits_));
					execute_op<LhsContainer, LhsContainer, Op>(op_lhs, op_rhs);
				}
			}
		}


	public:
		class BitRef
		{
		private:
			BitArray& origin_;
			uint64_t idx_;

		public:
			constexpr BitRef(BitArray& origin, uint64_t idx) :
				origin_(origin),
				idx_(idx)
			{ }

			inline constexpr operator bool() const
			{
				return origin_.get(idx_);
			}

			inline constexpr BitRef& operator=(const BitRef& other)
			{
				origin_.set(idx_, other);
				return *this;
			}

			inline constexpr BitRef& operator=(bool other)
			{
				origin_.set(idx_, other);
				return *this;
			}

			inline constexpr BitRef& operator=(BitRef&& other)
			{
				origin_.set(idx_, std::move(other));
				return *this;
			}
		};

		constexpr BitArray() :
			bit_chunks_{}
		{
			// memory::fill<Container, chunk_size_>(bit_chunks_, 0);
		}

		constexpr BitArray(const BitArray& other)
		{
			memory::copy<Container, chunk_size_>(this->bit_chunks_, other.bit_chunks_);
		}

		constexpr BitArray& operator=(const BitArray& other)
		{
			memory::copy<Container, chunk_size_>(this->bit_chunks_, other.bit_chunks_);
			return *this;
		}

		constexpr BitArray(BitArray&& other)
		{
			memory::copy<Container, chunk_size_>(this->bit_chunks_, other.bit_chunks_);
		}

		constexpr BitArray& operator=(BitArray&& other)
		{
			memory::copy<Container, chunk_size_>(this->bit_chunks_, other.bit_chunks_);
			return *this;
		}

		inline constexpr void increment(uint64_t chunk_start)
		{
			for (uint64_t i = chunk_start; i < chunk_size_; i++)
			{
				if (++bit_chunks_[i] != 0) break;
			}
		}

		inline constexpr BitArray& operator++(int)
		{
			increment(0);
			return *this;
		}

		inline constexpr bool get(uint64_t idx) const
		{
			return bit_chunks_[chunk(idx)] & (1ull << chunk_idx(idx));
		}

		inline constexpr BitRef operator[](uint64_t idx)
		{
			return BitRef(*this, idx);
		}

		inline constexpr void set(uint64_t idx, bool val)
		{
			if (val == true)
			{
				bit_chunks_[chunk(idx)] |= 1ull << chunk_idx(idx);
			}
			else
			{
				bit_chunks_[chunk(idx)] &= ~(1ull << chunk_idx(idx));
			}
		}

		inline constexpr void flip(uint64_t idx)
		{
			bit_chunks_[chunk(idx)] ^=  1ull << chunk_idx(idx);
		}

		template <typename OtherContainer, uint64_t OtherNumBits>
		inline constexpr bool operator==(const BitArray<OtherContainer, OtherNumBits>& other) const
		{
			if (NumBits != OtherNumBits) return false;
			return std::memcmp(bit_chunks_, other.bit_chunks_, total_bytes_);
		}

		inline constexpr uint64_t hash() const
		{
			uint64_t seed = 0;
			for (uint64_t i = 0; i < chunk_size_; i++)
			{
				Hasher::hash_combine(&seed, bit_chunks_[i]);
			}
			return seed;
		}

		inline constexpr uint64_t size() const
		{
			return NumBits;
		}

		constexpr std::string to_str() const
		{
			std::string str;
			str.reserve(NumBits);

			for (uint64_t i = NumBits - 1; i > 0; i--)
			{
				str += std::to_string(get(i));
			}
			str += std::to_string(get(0));

			return str;
		}

		constexpr BitArray& operator<<=(uint64_t shift)
		{
			uint64_t left_chunk = chunk(shift);
			uint64_t right_chunk = left_chunk + 1;
			uint64_t right_chunk_amount = chunk_idx(shift);
			uint64_t left_chunk_amount = container_bits_ - right_chunk_amount;

			if (right_chunk_amount == 0)
			{
				for (uint64_t i = chunk_size_ - 1; i != ~0ull; i--)
				{
					bit_chunks_[i] = left_chunk > i ? 0 : bit_chunks_[i - left_chunk];
				}
				return *this;
			}

			for (uint64_t i = chunk_size_ - 1; i != ~0ull; i--)
			{
				uint64_t shifted_chunk_left = left_chunk > i ? 0 : bit_chunks_[i - left_chunk] << right_chunk_amount;
				uint64_t shifted_chunk_right = right_chunk > i ? 0 : bit_chunks_[i - right_chunk] >> left_chunk_amount;
				bit_chunks_[i] = (shifted_chunk_left + shifted_chunk_right);
			}

			return *this;
		}

		constexpr BitArray& operator>>=(uint64_t shift)
		{
			uint64_t right_chunk = chunk(shift);
			uint64_t left_chunk = right_chunk + 1;
			uint64_t left_chunk_amount = chunk_idx(shift);
			uint64_t right_chunk_amount = container_bits_ - left_chunk_amount;

			if (left_chunk_amount == 0)
			{
				for (uint64_t i = 0; i < chunk_size_; i++)
				{
					bit_chunks_[i] = (i + right_chunk) >= chunk_size_ ? 0 : bit_chunks_[i + right_chunk];
				}
				return *this;
			}

			for (uint64_t i = 0; i < chunk_size_; i++)
			{
				uint64_t shifted_chunk_left = (i + left_chunk) >= chunk_size_ ? 0 : bit_chunks_[i + left_chunk] << right_chunk_amount;
				uint64_t shifted_chunk_right = (i + right_chunk) >= chunk_size_ ? 0 : bit_chunks_[i + right_chunk] >> left_chunk_amount;
				bit_chunks_[i] = (shifted_chunk_left + shifted_chunk_right);
			}

			return *this;
		}

		// only same container allowed for performance considerations
		template <uint64_t OtherNumBits>
		constexpr BitArray& operator+=(const BitArray<Container, OtherNumBits>& other)
		{
			constexpr uint64_t min_num_chunks = std::min(chunk_size_, other.chunk_size_);
			constexpr bool this_is_bigger = chunk_size_ > other.chunk_size_;

			uint64_t old_val;
			uint64_t new_val;
			uint64_t carry = 0;
			for(uint64_t i = 0; i < min_num_chunks; i++)
			{
				old_val = bit_chunks_[i];
				new_val = (bit_chunks_[i] += other.bit_chunks_[i] + carry);

				if (new_val < old_val)
				{
					carry = 1;
				}
				else
				{
					carry = 0;
				}
			}

			if (this_is_bigger)
			{
				increment(min_num_chunks);
			}

			return *this;
		}

		template <uint64_t OtherNumBits>
		constexpr BitArray& operator^=(const BitArray<Container, OtherNumBits>& other)
		{
			apply_operator<Container, NumBits, OtherNumBits, Operator::Xor>(*this, other);
			return *this;
		}

		template <uint64_t OtherNumBits>
		constexpr BitArray& operator|=(const BitArray<Container, OtherNumBits>& other)
		{
			apply_operator<Container, NumBits, OtherNumBits, Operator::Or>(*this, other);
			return *this;
		}

		template <uint64_t OtherNumBits>
		constexpr BitArray& operator&=(const BitArray<Container, OtherNumBits>& other)
		{
			apply_operator<Container, NumBits, OtherNumBits, Operator::And>(*this, other);
			return *this;
		}

		template <typename OtherContainer, uint64_t OtherNumBits>
		constexpr BitArray& operator|=(const BitArray<OtherContainer, OtherNumBits>& other)
		{
			apply_operator<Container, NumBits, OtherContainer, OtherNumBits, Operator::Or>(*this, other);
			return *this;
		}

		template <typename OtherContainer, uint64_t OtherNumBits>
		constexpr BitArray& operator&=(const BitArray<OtherContainer, OtherNumBits>& other)
		{
			apply_operator<Container, NumBits, OtherContainer, OtherNumBits, Operator::And>(*this, other);
			return *this;
		}

		template <typename OtherContainer, uint64_t OtherNumBits>
		constexpr BitArray& operator^=(const BitArray<OtherContainer, OtherNumBits>& other)
		{
			apply_operator<Container, NumBits, OtherContainer, OtherNumBits, Operator::Xor>(*this, other);
			return *this;
		}

		constexpr BitArray& operator~()
		{
			for(uint64_t i = 0; i < chunk_size_; i++)
			{
				bit_chunks_[i] = ~bit_chunks_[i];
			}

			return *this;
		}

		// assumes byte alignment of start and end
		template <typename T, uint64_t LenBits>
		inline constexpr void to_bits_fast_aligned(uint64_t obj_start_idx, const T& obj)
		{
			constexpr uint64_t len_bytes = LenBits >> 3; // len_bits / 8
			constexpr bool is_container_sized = sizeof(Container) == sizeof(T);

			if constexpr (is_container_sized)
			{
				// std::memcpy(bit_chunks_ + obj_start_idx, &obj, len_bytes);

				bit_chunks_[obj_start_idx] = *const_cast<Container*>(&obj);
				return;
			}

			uint64_t start_byte = (obj_start_idx * LenBits) >> 3;
			uint8_t* start = (uint8_t*)bit_chunks_ + start_byte;
			std::memcpy(start, &obj, len_bytes);
			// std::memcpy(start, (uint8_t*)&obj, len_bytes);
		}

		// clearing is inefficient
		template <typename T, bool Clear, uint64_t LenBits>
		constexpr void to_bits_fast(uint64_t start_idx, const T& obj)
		{
			static_assert(std::is_trivially_copyable_v<T>, "type must be trivially copyable for set_bits()");
			constexpr uint64_t len_bytes = LenBits >> 3; // len_bits / 8
			constexpr uint8_t overflow_bits = LenBits & 7; // len_bits % 8
			constexpr uint8_t overflow_mask = (overflow_bits == 0) ? 0 : 0xff >> (8 - overflow_bits);
			constexpr uint8_t extra_byte = (overflow_bits == 0) ? 0 : 1;
			constexpr uint64_t usable_obj_bytes = len_bytes + extra_byte;
			constexpr uint64_t true_obj_bytes = sizeof(T);
			if constexpr(true_obj_bytes < usable_obj_bytes) throw std::runtime_error("object is smaller than the specified bit length");

			uint8_t missaligned_bits_start = (start_idx & 7);
			uint64_t start_byte = (start_idx >> 3);
			uint8_t* start = (uint8_t*)bit_chunks_ + start_byte;

			uint64_t missaligned_bits_end = (start_idx + LenBits) & 7;
			uint64_t end_byte = (start_idx + LenBits) >> 3;
			uint8_t* end = (uint8_t*)bit_chunks_ + end_byte;

			bool aligned = (missaligned_bits_start == 0);

			if constexpr (Clear)
			{
				if (aligned)
				{
					std::memset(start, 0x00, len_bytes);
					if constexpr (overflow_bits > 0)
					{
						start[len_bytes] &= ~overflow_mask;
					}
				}
				else
				{
					uint8_t start_mask = (0xff << missaligned_bits_start);
					uint8_t end_mask = ((missaligned_bits_end == 0) ? 0 : (0xff >> (8 - missaligned_bits_end)));

					if (end_byte == start_byte)
					{
						start_mask &= end_mask;
						*start &= ~start_mask;
					}
					else
					{
						uint64_t aligned_bytes = end_byte - start_byte - 1;

						*start &= ~start_mask;
						std::memset(((uint8_t*)bit_chunks_) + start_byte + 1, 0x00, aligned_bytes);
						*end &= ~end_mask;
					}
				}
			}

			uint8_t* obj_start = (uint8_t*)&obj;

			if (aligned)
			{
				std::memcpy(start, obj_start, len_bytes);

				if constexpr (extra_byte > 0)
				{
					uint8_t end_bits = obj_start[len_bytes] & overflow_mask;
					start[len_bytes] |= end_bits;
				}
			}
			else
			{
				uint8_t start_bits = (*obj_start) << missaligned_bits_start;

				if (end_byte == start_byte)
				{
					uint8_t end_mask = 0xff >> (8 - missaligned_bits_end);
					*start |= start_bits & end_mask;
				}
				else if (usable_obj_bytes == 1)
				{
					uint8_t end_bits = (*obj_start) >> (8 - missaligned_bits_start);

					*start |= start_bits;
					*end |= end_bits;
				}
				else
				{
					// uint8_t offset = 8 - missaligned_bits_start;
					for (uint64_t i = 0; i < len_bytes; i++)
					{
						// start[i] |= obj_start[i] << missaligned_bits_start;
						// start[i + 1] |= obj_start[i] >> offset;
						uint16_t* curr = (uint16_t*)(start + i);
						*curr |= ((uint16_t)obj_start[i] << missaligned_bits_start);
					}

					if (extra_byte > 0)
					{
						uint64_t last_idx = len_bytes;
						// uint8_t last_val = obj_start[last_idx] & overflow_mask;
						// start[last_idx] |= last_val << missaligned_bits_start;
						// start[last_idx + 1] |= last_val >> offset;
						uint16_t last_val = obj_start[last_idx] & overflow_mask;
						uint16_t* curr = (uint16_t*)(start + last_idx);
						*curr |= last_val << missaligned_bits_start;
					}
				}
			}
		}

		template <typename T, bool Clear>
		constexpr void to_bits(uint64_t start, const T& obj)
		{
			static_assert(std::is_trivially_copyable_v<T>, "type must be trivially copyable for set_bits()");

			if constexpr (Clear)
			{
				BitArray<uint64_t, NumBits> mask;
				std::memset(mask.bit_chunks_, 0xff, sizeof(T));
				mask <<= start;
				*this &= ~mask;
			}

			BitArray<uint64_t, NumBits> obj_bits;
			std::memcpy(obj_bits.bit_chunks_, &obj, sizeof(T));
			obj_bits <<= start;
			*this |= obj_bits;
		}

		// assumes byte alignment of start and end
		// obj_start_idx is the index of the object, assuming all of the objects have LenBits
		template<typename T, uint64_t LenBits>
		inline constexpr T from_bits_fast_aligned(uint64_t obj_start_idx) const
		{
			constexpr uint64_t len_bytes = LenBits >> 3; // len_bits / 8
			constexpr uint64_t true_obj_bytes = sizeof(T);
			constexpr bool is_container_sized = sizeof(Container) == sizeof(T);

			if constexpr (is_container_sized)
			{
				// uint8_t obj[true_obj_bytes];
				// std::memcpy(obj, bit_chunks_ + obj_start_idx, true_obj_bytes);
				// return *reinterpret_cast<T*>(obj);

				// return *const_cast<Container*>(bit_chunks_ + obj_start_idx);
				return (T)*const_cast<Container*>(bit_chunks_ + obj_start_idx);
			}

			uint64_t start_byte = obj_start_idx * len_bytes;
			uint8_t* start = (uint8_t*)bit_chunks_ + start_byte;

			uint8_t obj[true_obj_bytes];
			if constexpr (true_obj_bytes > len_bytes)
			{
				std::memset(obj, 0x00, true_obj_bytes);
			}
			std::memcpy(obj, start, len_bytes);

			return *reinterpret_cast<T*>(obj);
		}

		template <typename T, uint64_t LenBits>
		constexpr T from_bits_fast(uint64_t start_idx) const
		{
			// PERF for efficiency, instead of using uint8_t, use maximum chunk size (tricky)
			// PERF for efficiency, do bit operations on the start/end directly instead of using masks (?)
			static_assert(std::is_trivially_copyable_v<T>, "type must be trivially copyable for set_bits()");
			constexpr uint64_t len_bytes = LenBits >> 3; // len_bits / 8
			constexpr uint64_t overflow_bits = LenBits & 7; // len_bits % 8
			constexpr uint8_t overflow_mask = (overflow_bits == 0) ? 0 : 0xff >> (8 - overflow_bits);
			constexpr uint8_t extra_byte = overflow_bits == 0 ? 0 : 1;
			constexpr uint64_t usable_obj_bytes = len_bytes + extra_byte;
			constexpr uint64_t true_obj_bytes = sizeof(T);
			if constexpr(true_obj_bytes < usable_obj_bytes) throw std::runtime_error("object is smaller than the specified bit length");

			uint64_t start_byte = (start_idx >> 3);
			uint8_t* start = (uint8_t*)bit_chunks_ + start_byte;
			uint64_t end_byte = (start_idx + LenBits) >> 3;
			uint8_t* end = (uint8_t*)bit_chunks_ + end_byte;

			uint8_t missaligned_bits_start = (start_idx & 7);
			uint8_t missaligned_bits_end = (start_idx + LenBits) & 7;
			uint8_t end_mask = ((missaligned_bits_end == 0) ? 0 : (0xff >> (8 - missaligned_bits_end)));

			uint8_t obj[true_obj_bytes];
			if constexpr (true_obj_bytes > len_bytes)
			{
				std::memset(obj, 0x00, true_obj_bytes);
			}

			bool aligned = (missaligned_bits_start == 0);
			if (aligned)
			{
				std::memcpy(obj, start, len_bytes);

				if constexpr (extra_byte > 0)
				{
					// uint8_t overflow_mask = 0xff >> (8 - overflow_bits);
					uint8_t end_bits = start[len_bytes] & overflow_mask;
					obj[len_bytes] = end_bits;
				}
			}
			else
			{
				if (end_byte == start_byte)
				{
					uint8_t start_mask = 0xff << missaligned_bits_start;
					uint8_t obj_mask = (end_mask == 0) ? start_mask : start_mask & end_mask;
					*obj = (*start & obj_mask) >> missaligned_bits_start;
				}
				else if (usable_obj_bytes == 1)
				{
					uint8_t start_bits = start[0] >> missaligned_bits_start;
					uint8_t end_bits = (*end) & end_mask;

					end_bits <<= (8 - missaligned_bits_start);
					*obj = (start_bits | end_bits);
				}
				else
				{
					// uint8_t offset = 8 - missaligned_bits_start;
					for (uint64_t i = 0; i < len_bytes; i++)
					{
						obj[i] = *((uint16_t*)(start + i)) >> missaligned_bits_start;
						// obj[i] = (start[i] >> missaligned_bits_start) | (start[i + 1] << offset);
					}

					if (extra_byte > 0)
					{
						uint64_t last_idx = len_bytes;
						obj[last_idx] = (*((uint16_t*)(start + last_idx)) >> missaligned_bits_start) & overflow_mask;
						// obj[last_idx] = ((start[last_idx] >> missaligned_bits_start) | ((start[last_idx + 1] & end_mask) << offset)) & overflow_mask;
					}
				}
			}

			return *reinterpret_cast<T*>(obj);
		}

		template <typename T>
		T from_bits(uint64_t start, uint64_t len_bits) const
		{
			static_assert(std::is_trivially_copyable_v<T>, "type must be trivially copyable for set_bits()");
			uint64_t len_bytes = len_bits >> 3; // len_bits / 8
			uint64_t overflow_bits = len_bits & 7; // len_bits % 8
			uint8_t overflow_mask = 0xff >> (8 - overflow_bits);

			BitArray<uint64_t, NumBits> mask;
			std::memset(mask.bit_chunks_, 0xff, len_bytes);

			uint8_t* mask_overflow_target = (uint8_t*)mask.bit_chunks_ + len_bytes;
			*mask_overflow_target |= overflow_mask;

			BitArray<Container, NumBits> obj_bits = *this;
			obj_bits >>= start;
			obj_bits &= mask;

			return *reinterpret_cast<T*>(obj_bits.bit_chunks_);
		}

		template <typename T>
		T from_bits(uint64_t start) const
		{
			static_assert(std::is_trivially_copyable_v<T>, "type must be trivially copyable for set_bits()");

			BitArray<uint64_t, NumBits> mask;
			std::memset(mask.bit_chunks_, 0xff, sizeof(T));

			BitArray<Container, NumBits> obj_bits = *this;
			obj_bits >>= start;
			obj_bits &= mask;

			return *reinterpret_cast<T*>(obj_bits.bit_chunks_);
		}

		constexpr bool cmp(const BitArray<Container, NumBits>& other, uint64_t start, uint64_t len) const
		{
			uint64_t start_byte = start >> 3;
			uint64_t start_bit = start & 7;
			uint8_t* this_start = (uint8_t*)bit_chunks_ + start_byte;
			uint8_t* other_start = (uint8_t*)other.bit_chunks_ + start_byte;

			uint64_t end_byte = (start + len) >> 3;
			uint64_t end_bit = (start + len) & 7;
			uint8_t* this_end = (uint8_t*)bit_chunks_ + end_byte;

			if (this_start == this_end)
			{
				uint8_t left_shift = end_bit == 0 ? 0 : (8 - end_bit);
				uint8_t right_shift = left_shift + start_bit;
				uint8_t this_byte = (*this_start << left_shift) >> right_shift;
				uint8_t other_byte = (*other_start << left_shift) >> right_shift;
				return this_byte == other_byte;
			}

			bool misaligned_start = start_bit != 0;
			if (misaligned_start)
			{
				uint8_t this_byte = *this_start >> start_bit;
				uint8_t other_byte = *other_start >> start_bit;
				if (this_byte != other_byte)
				{
					return false;
				}
			}

			if (std::memcmp(this_start, other_start, end_byte - start_byte))
			{
				return false;
			}

			uint8_t* other_end = (uint8_t*)other.bit_chunks_ + end_byte;

			bool misaligned_end = end_bit != 0;
			if (misaligned_end)
			{
				uint8_t left_shift = 8 - end_bit;
				uint8_t this_byte = *this_end << left_shift;
				uint8_t other_byte = *other_end << left_shift;
				if (this_byte != other_byte)
				{
					return false;
				}
			}

			return true;
		}

		// TODO
		// template<typename OtherContainer, uint64_t OtherNumBits>
		// constexpr bool cmp(const BitArray<OtherContainer, OtherNumBits>& other, uint64_t start_bit, uint64_t other_start_bit, uint64_t len) const
		// {
			// uint64_t this_start_byte = start_bit >> 3;
			// uint64_t this_start_bit_offset = start_bit & 7;
			// uint8_t* this_start = (uint8_t*)bit_chunks_ + this_start_byte;
			//
			// uint64_t other_start_byte = other_start_bit >> 3;
			// uint64_t other_start_bit_offset = other_start_bit & 7;
			// uint8_t* other_start = (uint8_t*)other.bit_chunks_ + other_start_byte;
			//
			// uint64_t this_end_byte = (start_bit + len) >> 3;
			// uint64_t this_end_bit_offset = (start_bit + len) & 7;
			// uint8_t* this_end = (uint8_t*)bit_chunks_ + this_end_byte;
			//
			// if (this_start == this_end)
			// {
			// 	uint8_t left_shift = this_end_bit_offset == 0 ? 0 : (8 - this_end_bit_offset);
			// 	uint8_t right_shift = left_shift + this_start_bit_offset;
			// 	uint8_t this_byte = (*this_start << left_shift) >> right_shift;
			// 	uint8_t other_byte = (*other_start << left_shift) >> right_shift;
			// 	return this_byte == other_byte;
			// }
			//
			// bool misaligned_start = this_start_bit_offset != 0 || other_start_bit_offset != 0;
			// if (misaligned_start)
			// {
			// 	uint8_t this_byte = *this_start >> this_start_bit_offset;
			// 	uint8_t other_byte = *other_start >> this_start_bit_offset;
			// 	if (this_byte != other_byte)
			// 	{
			// 		return false;
			// 	}
			// }
			//
			// if (std::memcmp(this_start, other_start, this_end_byte - this_start_byte))
			// {
			// 	return false;
			// }
			//
			// uint8_t* other_end = (uint8_t*)other.bit_chunks_ + this_end_byte;
			//
			// bool misaligned_end = this_end_bit_offset != 0;
			// if (misaligned_end)
			// {
			// 	uint8_t left_shift = 8 - this_end_bit_offset;
			// 	uint8_t this_byte = *this_end << left_shift;
			// 	uint8_t other_byte = *other_end << left_shift;
			// 	if (this_byte != other_byte)
			// 	{
			// 		return false;
			// 	}
			// }
			//
			// return true;
		// }
	};

}

