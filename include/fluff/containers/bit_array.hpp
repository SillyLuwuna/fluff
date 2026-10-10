#ifndef FLUFF_CONTAINERS_BIT_ARRAY_HPP
#define FLUFF_CONTAINERS_BIT_ARRAY_HPP

#include "fluff/hashing/hasher.hpp"
#include "fluff/memory/memory.hpp"
#include "fluff/types.hpp"
#include <cstring>
#include <stdexcept>
#include <string>

namespace fluff
{
	using namespace fluff;

	template <typename Container, u64 NumBits>
	class BitArray
	{
	private:
		template<typename OtherContainer, u64 OtherNumBits>
		friend class BitArray;

		static constexpr u64 container_bytes_ = sizeof(Container);
		static constexpr u64 container_bits_ = container_bytes_ * 8;
		static constexpr u64 container_division_ = std::bit_width(container_bits_) - 1; // log2
		static constexpr u64 container_modulus_ = container_bits_ - 1;

		static constexpr u64 chunk_size_ = ((NumBits - 1) / container_bits_) + 1;
		static constexpr u64 total_bytes_ = container_bytes_ * chunk_size_;

		Container bit_chunks_[chunk_size_];

		static inline constexpr u64 chunk(u64 idx)
		{
			return idx >> container_division_;
		}

		static inline constexpr u64 chunk_idx(u64 idx)
		{
			return idx & container_modulus_;
		}

		enum Operator : u8
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
			else throw std::runtime_error("Unknown bit array operation"); // PERF remove exceptions
		}

		template <typename GeneralContainer, u64 LhsNumBits, u64 RhsNumBits, Operator Op>
		static inline constexpr void apply_operator(BitArray<GeneralContainer, LhsNumBits>& lhs, const BitArray<GeneralContainer, RhsNumBits>& rhs)
		{
			constexpr u64 min_num_chunks = std::min(lhs.chunk_size_, rhs.chunk_size_);

			for(u64 i = 0; i < min_num_chunks; i++)
			{
				execute_op(lhs.bit_chunks_ + i, rhs.bit_chunks_[i]);
			}
		}

		template <typename LhsContainer, u64 LhsNumBits, typename RhsContainer, u64 RhsNumBits, Operator Op>
		static constexpr void apply_operator(BitArray<LhsContainer, LhsNumBits>& lhs, const BitArray<RhsContainer, RhsNumBits>& rhs)
		{
			using Lhs = BitArray<LhsContainer, LhsNumBits>;
			using Rhs = BitArray<RhsContainer, RhsNumBits>;
			constexpr bool is_lhs_larger = Lhs::container_bytes_ > Rhs::container_bytes_;
			constexpr u8 byte_ratio = is_lhs_larger ? (Lhs::container_bytes_ / Rhs::container_bytes_) : (Rhs::container_bytes_ / Lhs::container_bytes_);
			constexpr u64 chunks_aligned = is_lhs_larger ? (Rhs::total_bytes_ / Lhs::container_bytes_) : (Lhs::total_bytes_ / Rhs::container_bytes_);
			constexpr u8 bytes_missaligned = is_lhs_larger ? (Rhs::total_bytes_ % Lhs::container_bytes_) : (Lhs::total_bytes_ % Rhs::container_bytes_);
			constexpr u8 chunks_missaligned = is_lhs_larger ? (bytes_missaligned / Rhs::container_bytes_) : (bytes_missaligned / Lhs::container_bytes_);

			for(u64 i = 0; i < chunks_aligned; i++)
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

			u64 curr_minor_chunk = 0;
			for (u64 i = chunks_aligned * byte_ratio; i < (chunks_missaligned + (chunks_aligned * byte_ratio)); i++)
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
			u64 idx_;

		public:
			constexpr BitRef(BitArray& origin, u64 idx) :
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
			// Mem::fill<Container, chunk_size_>(bit_chunks_, 0);
		}

		constexpr BitArray(const BitArray& other)
		{
			Mem::copy<Container, chunk_size_>(this->bit_chunks_, other.bit_chunks_);
		}

		constexpr BitArray& operator=(const BitArray& other)
		{
			Mem::copy<Container, chunk_size_>(this->bit_chunks_, other.bit_chunks_);
			return *this;
		}

		constexpr BitArray(BitArray&& other)
		{
			Mem::copy<Container, chunk_size_>(this->bit_chunks_, other.bit_chunks_);
		}

		constexpr BitArray& operator=(BitArray&& other)
		{
			Mem::copy<Container, chunk_size_>(this->bit_chunks_, other.bit_chunks_);
			return *this;
		}

		inline constexpr void increment(u64 chunk_start)
		{
			for (u64 i = chunk_start; i < chunk_size_; i++)
			{
				if (++bit_chunks_[i] != 0) break;
			}
		}

		inline constexpr BitArray& operator++(i32)
		{
			increment(0);
			return *this;
		}

		inline constexpr bool get(u64 idx) const
		{
			return bit_chunks_[chunk(idx)] & (1ull << chunk_idx(idx));
		}

		inline constexpr BitRef operator[](u64 idx)
		{
			return BitRef(*this, idx);
		}

		inline constexpr void set(u64 idx, bool val)
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

		inline constexpr void flip(u64 idx)
		{
			bit_chunks_[chunk(idx)] ^=  1ull << chunk_idx(idx);
		}

		template <typename OtherContainer, u64 OtherNumBits>
		inline constexpr bool operator==(const BitArray<OtherContainer, OtherNumBits>& other) const
		{
			if (NumBits != OtherNumBits) return false;
			return std::memcmp(bit_chunks_, other.bit_chunks_, total_bytes_);
		}

		inline constexpr u64 hash() const
		{
			u64 seed = 0;
			for (u64 i = 0; i < chunk_size_; i++)
			{
				Hasher::hash_combine(&seed, bit_chunks_[i]);
			}
			return seed;
		}

		inline constexpr u64 size() const
		{
			return NumBits;
		}

		constexpr std::string to_str() const
		{
			std::string str;
			str.reserve(NumBits);

			for (u64 i = NumBits - 1; i > 0; i--)
			{
				str += std::to_string(get(i));
			}
			str += std::to_string(get(0));

			return str;
		}

		constexpr BitArray& operator<<=(u64 shift)
		{
			u64 left_chunk = chunk(shift);
			u64 right_chunk = left_chunk + 1;
			u64 right_chunk_amount = chunk_idx(shift);
			u64 left_chunk_amount = container_bits_ - right_chunk_amount;

			if (right_chunk_amount == 0)
			{
				for (u64 i = chunk_size_ - 1; i != ~0ull; i--)
				{
					bit_chunks_[i] = left_chunk > i ? 0 : bit_chunks_[i - left_chunk];
				}
				return *this;
			}

			for (u64 i = chunk_size_ - 1; i != ~0ull; i--)
			{
				u64 shifted_chunk_left = left_chunk > i ? 0 : bit_chunks_[i - left_chunk] << right_chunk_amount;
				u64 shifted_chunk_right = right_chunk > i ? 0 : bit_chunks_[i - right_chunk] >> left_chunk_amount;
				bit_chunks_[i] = (shifted_chunk_left + shifted_chunk_right);
			}

			return *this;
		}

		constexpr BitArray& operator>>=(u64 shift)
		{
			u64 right_chunk = chunk(shift);
			u64 left_chunk = right_chunk + 1;
			u64 left_chunk_amount = chunk_idx(shift);
			u64 right_chunk_amount = container_bits_ - left_chunk_amount;

			if (left_chunk_amount == 0)
			{
				for (u64 i = 0; i < chunk_size_; i++)
				{
					bit_chunks_[i] = (i + right_chunk) >= chunk_size_ ? 0 : bit_chunks_[i + right_chunk];
				}
				return *this;
			}

			for (u64 i = 0; i < chunk_size_; i++)
			{
				u64 shifted_chunk_left = (i + left_chunk) >= chunk_size_ ? 0 : bit_chunks_[i + left_chunk] << right_chunk_amount;
				u64 shifted_chunk_right = (i + right_chunk) >= chunk_size_ ? 0 : bit_chunks_[i + right_chunk] >> left_chunk_amount;
				bit_chunks_[i] = (shifted_chunk_left + shifted_chunk_right);
			}

			return *this;
		}

		// only same container allowed for performance considerations
		template <u64 OtherNumBits>
		constexpr BitArray& operator+=(const BitArray<Container, OtherNumBits>& other)
		{
			constexpr u64 min_num_chunks = std::min(chunk_size_, other.chunk_size_);
			constexpr bool this_is_bigger = chunk_size_ > other.chunk_size_;

			u64 old_val;
			u64 new_val;
			u64 carry = 0;
			for(u64 i = 0; i < min_num_chunks; i++)
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

		template <u64 OtherNumBits>
		constexpr BitArray& operator^=(const BitArray<Container, OtherNumBits>& other)
		{
			apply_operator<Container, NumBits, OtherNumBits, Operator::Xor>(*this, other);
			return *this;
		}

		template <u64 OtherNumBits>
		constexpr BitArray& operator|=(const BitArray<Container, OtherNumBits>& other)
		{
			apply_operator<Container, NumBits, OtherNumBits, Operator::Or>(*this, other);
			return *this;
		}

		template <u64 OtherNumBits>
		constexpr BitArray& operator&=(const BitArray<Container, OtherNumBits>& other)
		{
			apply_operator<Container, NumBits, OtherNumBits, Operator::And>(*this, other);
			return *this;
		}

		template <typename OtherContainer, u64 OtherNumBits>
		constexpr BitArray& operator|=(const BitArray<OtherContainer, OtherNumBits>& other)
		{
			apply_operator<Container, NumBits, OtherContainer, OtherNumBits, Operator::Or>(*this, other);
			return *this;
		}

		template <typename OtherContainer, u64 OtherNumBits>
		constexpr BitArray& operator&=(const BitArray<OtherContainer, OtherNumBits>& other)
		{
			apply_operator<Container, NumBits, OtherContainer, OtherNumBits, Operator::And>(*this, other);
			return *this;
		}

		template <typename OtherContainer, u64 OtherNumBits>
		constexpr BitArray& operator^=(const BitArray<OtherContainer, OtherNumBits>& other)
		{
			apply_operator<Container, NumBits, OtherContainer, OtherNumBits, Operator::Xor>(*this, other);
			return *this;
		}

		constexpr BitArray& operator~()
		{
			for(u64 i = 0; i < chunk_size_; i++)
			{
				bit_chunks_[i] = ~bit_chunks_[i];
			}

			return *this;
		}

		// assumes byte alignment of start and end
		template <typename T, u64 LenBits>
		inline constexpr void to_bits_fast_aligned(u64 obj_start_idx, const T& obj)
		{
			constexpr u64 len_bytes = LenBits >> 3; // len_bits / 8
			constexpr bool is_container_sized = sizeof(Container) == sizeof(T);

			if constexpr (is_container_sized)
			{
				// std::memcpy(bit_chunks_ + obj_start_idx, &obj, len_bytes);

				bit_chunks_[obj_start_idx] = *const_cast<Container*>(&obj);
				return;
			}

			u64 start_byte = (obj_start_idx * LenBits) >> 3;
			u8* start = (u8*)bit_chunks_ + start_byte;
			std::memcpy(start, &obj, len_bytes);
			// std::memcpy(start, (u8*)&obj, len_bytes);
		}

		// clearing is inefficient
		template <typename T, bool Clear, u64 LenBits>
		constexpr void to_bits_fast(u64 start_idx, const T& obj)
		{
			static_assert(std::is_trivially_copyable_v<T>, "type must be trivially copyable for set_bits()");
			constexpr u64 len_bytes = LenBits >> 3; // len_bits / 8
			constexpr u8 overflow_bits = LenBits & 7; // len_bits % 8
			constexpr u8 overflow_mask = (overflow_bits == 0) ? 0 : 0xff >> (8 - overflow_bits);
			constexpr u8 extra_byte = (overflow_bits == 0) ? 0 : 1;
			constexpr u64 usable_obj_bytes = len_bytes + extra_byte;
			constexpr u64 true_obj_bytes = sizeof(T);
			if constexpr(true_obj_bytes < usable_obj_bytes) throw std::runtime_error("object is smaller than the specified bit length"); // PERF remove exceptions

			u8 missaligned_bits_start = (start_idx & 7);
			u64 start_byte = (start_idx >> 3);
			u8* start = (u8*)bit_chunks_ + start_byte;

			u64 missaligned_bits_end = (start_idx + LenBits) & 7;
			u64 end_byte = (start_idx + LenBits) >> 3;
			u8* end = (u8*)bit_chunks_ + end_byte;

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
					u8 start_mask = (0xff << missaligned_bits_start);
					u8 end_mask = ((missaligned_bits_end == 0) ? 0 : (0xff >> (8 - missaligned_bits_end)));

					if (end_byte == start_byte)
					{
						start_mask &= end_mask;
						*start &= ~start_mask;
					}
					else
					{
						u64 aligned_bytes = end_byte - start_byte - 1;

						*start &= ~start_mask;
						std::memset(((u8*)bit_chunks_) + start_byte + 1, 0x00, aligned_bytes);
						*end &= ~end_mask;
					}
				}
			}

			u8* obj_start = (u8*)&obj;

			if (aligned)
			{
				std::memcpy(start, obj_start, len_bytes);

				if constexpr (extra_byte > 0)
				{
					u8 end_bits = obj_start[len_bytes] & overflow_mask;
					start[len_bytes] |= end_bits;
				}
			}
			else
			{
				u8 start_bits = (*obj_start) << missaligned_bits_start;

				if (end_byte == start_byte)
				{
					u8 end_mask = 0xff >> (8 - missaligned_bits_end);
					*start |= start_bits & end_mask;
				}
				else if (usable_obj_bytes == 1)
				{
					u8 end_bits = (*obj_start) >> (8 - missaligned_bits_start);

					*start |= start_bits;
					*end |= end_bits;
				}
				else
				{
					// u8 offset = 8 - missaligned_bits_start;
					for (u64 i = 0; i < len_bytes; i++)
					{
						// start[i] |= obj_start[i] << missaligned_bits_start;
						// start[i + 1] |= obj_start[i] >> offset;
						u16* curr = (u16*)(start + i);
						*curr |= ((u16)obj_start[i] << missaligned_bits_start);
					}

					if (extra_byte > 0)
					{
						u64 last_idx = len_bytes;
						// u8 last_val = obj_start[last_idx] & overflow_mask;
						// start[last_idx] |= last_val << missaligned_bits_start;
						// start[last_idx + 1] |= last_val >> offset;
						u16 last_val = obj_start[last_idx] & overflow_mask;
						u16* curr = (u16*)(start + last_idx);
						*curr |= last_val << missaligned_bits_start;
					}
				}
			}
		}

		template <typename T, bool Clear>
		constexpr void to_bits(u64 start, const T& obj)
		{
			static_assert(std::is_trivially_copyable_v<T>, "type must be trivially copyable for set_bits()");

			if constexpr (Clear)
			{
				BitArray<u64, NumBits> mask;
				std::memset(mask.bit_chunks_, 0xff, sizeof(T));
				mask <<= start;
				*this &= ~mask;
			}

			BitArray<u64, NumBits> obj_bits;
			std::memcpy(obj_bits.bit_chunks_, &obj, sizeof(T));
			obj_bits <<= start;
			*this |= obj_bits;
		}

		// assumes byte alignment of start and end
		// obj_start_idx is the index of the object, assuming all of the objects have LenBits
		template<typename T, u64 LenBits>
		inline constexpr T from_bits_fast_aligned(u64 obj_start_idx) const
		{
			constexpr u64 len_bytes = LenBits >> 3; // len_bits / 8
			constexpr u64 true_obj_bytes = sizeof(T);
			constexpr bool is_container_sized = sizeof(Container) == sizeof(T);

			if constexpr (is_container_sized)
			{
				// u8 obj[true_obj_bytes];
				// std::memcpy(obj, bit_chunks_ + obj_start_idx, true_obj_bytes);
				// return *reinterpret_cast<T*>(obj);

				// return *const_cast<Container*>(bit_chunks_ + obj_start_idx);
				return (T)*const_cast<Container*>(bit_chunks_ + obj_start_idx);
			}

			u64 start_byte = obj_start_idx * len_bytes;
			u8* start = (u8*)bit_chunks_ + start_byte;

			u8 obj[true_obj_bytes];
			if constexpr (true_obj_bytes > len_bytes)
			{
				std::memset(obj, 0x00, true_obj_bytes);
			}
			std::memcpy(obj, start, len_bytes);

			return *reinterpret_cast<T*>(obj);
		}

		template <typename T, u64 LenBits>
		constexpr T from_bits_fast(u64 start_idx) const
		{
			// PERF for efficiency, instead of using u8, use maximum chunk size (tricky)
			// PERF for efficiency, do bit operations on the start/end directly instead of using masks (?)
			static_assert(std::is_trivially_copyable_v<T>, "type must be trivially copyable for set_bits()");
			constexpr u64 len_bytes = LenBits >> 3; // len_bits / 8
			constexpr u64 overflow_bits = LenBits & 7; // len_bits % 8
			constexpr u8 overflow_mask = (overflow_bits == 0) ? 0 : 0xff >> (8 - overflow_bits);
			constexpr u8 extra_byte = overflow_bits == 0 ? 0 : 1;
			constexpr u64 usable_obj_bytes = len_bytes + extra_byte;
			constexpr u64 true_obj_bytes = sizeof(T);
			if constexpr(true_obj_bytes < usable_obj_bytes) throw std::runtime_error("object is smaller than the specified bit length"); // PERF remove exceptions

			u64 start_byte = (start_idx >> 3);
			u8* start = (u8*)bit_chunks_ + start_byte;
			u64 end_byte = (start_idx + LenBits) >> 3;
			u8* end = (u8*)bit_chunks_ + end_byte;

			u8 missaligned_bits_start = (start_idx & 7);
			u8 missaligned_bits_end = (start_idx + LenBits) & 7;
			u8 end_mask = ((missaligned_bits_end == 0) ? 0 : (0xff >> (8 - missaligned_bits_end)));

			u8 obj[true_obj_bytes];
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
					// u8 overflow_mask = 0xff >> (8 - overflow_bits);
					u8 end_bits = start[len_bytes] & overflow_mask;
					obj[len_bytes] = end_bits;
				}
			}
			else
			{
				if (end_byte == start_byte)
				{
					u8 start_mask = 0xff << missaligned_bits_start;
					u8 obj_mask = (end_mask == 0) ? start_mask : start_mask & end_mask;
					*obj = (*start & obj_mask) >> missaligned_bits_start;
				}
				else if (usable_obj_bytes == 1)
				{
					u8 start_bits = start[0] >> missaligned_bits_start;
					u8 end_bits = (*end) & end_mask;

					end_bits <<= (8 - missaligned_bits_start);
					*obj = (start_bits | end_bits);
				}
				else
				{
					// u8 offset = 8 - missaligned_bits_start;
					for (u64 i = 0; i < len_bytes; i++)
					{
						obj[i] = *((u16*)(start + i)) >> missaligned_bits_start;
						// obj[i] = (start[i] >> missaligned_bits_start) | (start[i + 1] << offset);
					}

					if (extra_byte > 0)
					{
						u64 last_idx = len_bytes;
						obj[last_idx] = (*((u16*)(start + last_idx)) >> missaligned_bits_start) & overflow_mask;
						// obj[last_idx] = ((start[last_idx] >> missaligned_bits_start) | ((start[last_idx + 1] & end_mask) << offset)) & overflow_mask;
					}
				}
			}

			return *reinterpret_cast<T*>(obj);
		}

		template <typename T>
		T from_bits(u64 start, u64 len_bits) const
		{
			static_assert(std::is_trivially_copyable_v<T>, "type must be trivially copyable for set_bits()");
			u64 len_bytes = len_bits >> 3; // len_bits / 8
			u64 overflow_bits = len_bits & 7; // len_bits % 8
			u8 overflow_mask = 0xff >> (8 - overflow_bits);

			BitArray<u64, NumBits> mask;
			std::memset(mask.bit_chunks_, 0xff, len_bytes);

			u8* mask_overflow_target = (u8*)mask.bit_chunks_ + len_bytes;
			*mask_overflow_target |= overflow_mask;

			BitArray<Container, NumBits> obj_bits = *this;
			obj_bits >>= start;
			obj_bits &= mask;

			return *reinterpret_cast<T*>(obj_bits.bit_chunks_);
		}

		template <typename T>
		T from_bits(u64 start) const
		{
			static_assert(std::is_trivially_copyable_v<T>, "type must be trivially copyable for set_bits()");

			BitArray<u64, NumBits> mask;
			std::memset(mask.bit_chunks_, 0xff, sizeof(T));

			BitArray<Container, NumBits> obj_bits = *this;
			obj_bits >>= start;
			obj_bits &= mask;

			return *reinterpret_cast<T*>(obj_bits.bit_chunks_);
		}

		constexpr bool cmp(const BitArray<Container, NumBits>& other, u64 start, u64 len) const
		{
			u64 start_byte = start >> 3;
			u64 start_bit = start & 7;
			u8* this_start = (u8*)bit_chunks_ + start_byte;
			u8* other_start = (u8*)other.bit_chunks_ + start_byte;

			u64 end_byte = (start + len) >> 3;
			u64 end_bit = (start + len) & 7;
			u8* this_end = (u8*)bit_chunks_ + end_byte;

			if (this_start == this_end)
			{
				u8 left_shift = end_bit == 0 ? 0 : (8 - end_bit);
				u8 right_shift = left_shift + start_bit;
				u8 this_byte = (*this_start << left_shift) >> right_shift;
				u8 other_byte = (*other_start << left_shift) >> right_shift;
				return this_byte == other_byte;
			}

			bool misaligned_start = start_bit != 0;
			if (misaligned_start)
			{
				u8 this_byte = *this_start >> start_bit;
				u8 other_byte = *other_start >> start_bit;
				if (this_byte != other_byte)
				{
					return false;
				}
			}

			if (std::memcmp(this_start, other_start, end_byte - start_byte))
			{
				return false;
			}

			u8* other_end = (u8*)other.bit_chunks_ + end_byte;

			bool misaligned_end = end_bit != 0;
			if (misaligned_end)
			{
				u8 left_shift = 8 - end_bit;
				u8 this_byte = *this_end << left_shift;
				u8 other_byte = *other_end << left_shift;
				if (this_byte != other_byte)
				{
					return false;
				}
			}

			return true;
		}

		// TODO
		// template<typename OtherContainer, u64 OtherNumBits>
		// constexpr bool cmp(const BitArray<OtherContainer, OtherNumBits>& other, u64 start_bit, u64 other_start_bit, u64 len) const
		// {
			// u64 this_start_byte = start_bit >> 3;
			// u64 this_start_bit_offset = start_bit & 7;
			// u8* this_start = (u8*)bit_chunks_ + this_start_byte;
			//
			// u64 other_start_byte = other_start_bit >> 3;
			// u64 other_start_bit_offset = other_start_bit & 7;
			// u8* other_start = (u8*)other.bit_chunks_ + other_start_byte;
			//
			// u64 this_end_byte = (start_bit + len) >> 3;
			// u64 this_end_bit_offset = (start_bit + len) & 7;
			// u8* this_end = (u8*)bit_chunks_ + this_end_byte;
			//
			// if (this_start == this_end)
			// {
			// 	u8 left_shift = this_end_bit_offset == 0 ? 0 : (8 - this_end_bit_offset);
			// 	u8 right_shift = left_shift + this_start_bit_offset;
			// 	u8 this_byte = (*this_start << left_shift) >> right_shift;
			// 	u8 other_byte = (*other_start << left_shift) >> right_shift;
			// 	return this_byte == other_byte;
			// }
			//
			// bool misaligned_start = this_start_bit_offset != 0 || other_start_bit_offset != 0;
			// if (misaligned_start)
			// {
			// 	u8 this_byte = *this_start >> this_start_bit_offset;
			// 	u8 other_byte = *other_start >> this_start_bit_offset;
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
			// u8* other_end = (u8*)other.bit_chunks_ + this_end_byte;
			//
			// bool misaligned_end = this_end_bit_offset != 0;
			// if (misaligned_end)
			// {
			// 	u8 left_shift = 8 - this_end_bit_offset;
			// 	u8 this_byte = *this_end << left_shift;
			// 	u8 other_byte = *other_end << left_shift;
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


#endif
