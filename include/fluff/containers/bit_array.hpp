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
	// TODO define overflow mechanics (?)
	// TODO + and - overloads, as well as -=
	// TODO numeric operations to BitArrays of different containers
	// TODO cmp for BitArrays of different container types
	using namespace fluff::types;
	using namespace fluff;

	/**
	 * Array of bits, having bit operations, numerical operations, and
	 * object insertion/extraction at arbitrary bit index.
	 *
	 * The class assumes correct use, not providing exceptions.
	 *
	 * Note: might not work for different indianness.
	 *
	 * @param Container the type of the buckets in which the bits will be stored.
	 * Choose uint64_t for maximum performance. Choose uint8_t for maximum memory efficiency.
	 * If Container matches the object being inserted/extracted, it is analogous to an array in
	 * both performance and memory efficiency.
	 *
	 * @param NumBits the desired number of bits in the array.
	 */
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

		/**
		 * Get the chunk in which the bit resides.
		 * @param idx the bit to check.
		 */
		static inline constexpr u64 chunk(u64 idx)
		{
			return idx >> container_division_;
		}

		/**
		 * Get the index on the chunk in which the bit resides.
		 * @param idx the bit to check.
		 */
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

		/**
		 * Execute an abstract operation between two containers.
		 * Note that both containers may have different bit widths.
		 * @param LhsContainer the lhs container type.
		 * @param RhsContainer the rhs container type.
		 * @param Op the operation to execute.
		 * @param op_lhs the lhs container, where the result will also be written to.
		 * @param op_rhs the rhs container
		 */
		template <typename LhsContainer, typename RhsContainer, Operator Op>
		static inline constexpr void execute_op(LhsContainer* op_lhs, RhsContainer op_rhs)
		{
			if constexpr (Op == Operator::Or) *op_lhs |= op_rhs;
			else if constexpr (Op == Operator::And) *op_lhs &= op_rhs;
			else if constexpr (Op == Operator::Xor) *op_lhs ^= op_rhs;
			else throw std::runtime_error("Unknown bit array operation"); // PERF remove exceptions
		}

		/**
		 * Applies an abstract operation between two BitArrays with the same container.
		 * Given that this is more efficient that with two BitArrays with different containers.
		 * @param GeneralContainer the common container type between the two BitArrays.
		 * @param LhsNumBits the number of bits in the lhs BitArray.
		 * @param RhsNumBits the number of bits in the rhs BitArray.
		 * @param Op the operation to execute.
		 * @param lhs the lhs BitArray.
		 * @param rhs the rhs BitArray.
		 */
		template <typename GeneralContainer, u64 LhsNumBits, u64 RhsNumBits, Operator Op>
		static inline constexpr void apply_operator(BitArray<GeneralContainer, LhsNumBits>& lhs, const BitArray<GeneralContainer, RhsNumBits>& rhs)
		{
			constexpr u64 min_num_chunks = std::min(lhs.chunk_size_, rhs.chunk_size_);

			for(u64 i = 0; i < min_num_chunks; i++)
			{
				execute_op(lhs.bit_chunks_ + i, rhs.bit_chunks_[i]);
			}
		}

		/**
		 * Applies an abstract operation between two BitArrays.
		 * This is more inefficient than if both BitArrays have the same container.
		 * @param LhsContainer the container type in the lhs BitArray.
		 * @param LhsNumBits the number of bits in the lhs BitArray.
		 * @param RhsContainer the container type in the rhs BitArray.
		 * @param RhsNumBits the number of bits in the rhs BitArray.
		 * @param Op the operation to execute.
		 * @param lhs the lhs BitArray.
		 * @param rhs the rhs BitArray.
		 */
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
		/**
		 * A bit reference that symbolizes a bit from a BitArray.
		 * Used for operator overloads such as [].
		 */
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

			/**
			 * Bool conversion of the bit value, so it can
			 * be used as a bool.
			 */
			inline constexpr operator bool() const
			{
				return origin_.get(idx_);
			}

			/**
			 * Assign the value of one bit to another.
			 */
			inline constexpr BitRef& operator=(const BitRef& other)
			{
				origin_.set(idx_, other);
				return *this;
			}

			/**
			 * Assign the value of one bit to a value.
			 */
			inline constexpr BitRef& operator=(bool other)
			{
				origin_.set(idx_, other);
				return *this;
			}

			/**
			 * Move the value of one bit to another.
			 */
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
			fluff::memory::copy<Container, chunk_size_>(this->bit_chunks_, other.bit_chunks_);
		}

		constexpr BitArray& operator=(const BitArray& other)
		{
			fluff::memory::copy<Container, chunk_size_>(this->bit_chunks_, other.bit_chunks_);
			return *this;
		}

		constexpr BitArray(BitArray&& other)
		{
			fluff::memory::copy<Container, chunk_size_>(this->bit_chunks_, other.bit_chunks_);
		}

		constexpr BitArray& operator=(BitArray&& other)
		{
			fluff::memory::copy<Container, chunk_size_>(this->bit_chunks_, other.bit_chunks_);
			return *this;
		}

		/**
		 * Increments the value in the whole BitArray by 1.
		 */
		inline constexpr void increment(u64 chunk_start)
		{
			for (u64 i = chunk_start; i < chunk_size_; i++)
			{
				if (++bit_chunks_[i] != 0) break;
			}
		}

		/**
		 * Increments the value in the whole BitArray by 1.
		 */
		inline constexpr BitArray& operator++(i32)
		{
			increment(0);
			return *this;
		}

		/**
		 * Gets the bit at the given index.
		 */
		inline constexpr bool get(u64 idx) const
		{
			return bit_chunks_[chunk(idx)] & (1ull << chunk_idx(idx));
		}

		/**
		 * Gets the bit at the given index.
		 */
		inline constexpr BitRef operator[](u64 idx)
		{
			return BitRef(*this, idx);
		}

		/**
		 * Sets the bit at idx to val.
		 */
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

		/**
		 * Flips the bit at the given index.
		 */
		inline constexpr void flip(u64 idx)
		{
			bit_chunks_[chunk(idx)] ^=  1ull << chunk_idx(idx);
		}

		/**
		 * Checks for equality between two BitArrays.
		 */
		template <typename OtherContainer, u64 OtherNumBits>
		inline constexpr bool operator==(const BitArray<OtherContainer, OtherNumBits>& other) const
		{
			if (NumBits != OtherNumBits) return false;
			return std::memcmp(bit_chunks_, other.bit_chunks_, total_bytes_);
		}

		/**
		 * Returns the hash of the BitArray.
		 * Note: inefficient.
		 */
		inline constexpr u64 hash() const
		{
			u64 seed = 0;
			for (u64 i = 0; i < chunk_size_; i++)
			{
				fluff::hashing::hash_combine(&seed, bit_chunks_[i]);
			}
			return seed;
		}

		/**
		 * The number of bits in the BitArray.
		 */
		inline constexpr u64 size() const
		{
			return NumBits;
		}

		/**
		 * A string representation of the BitArray.
		 */
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

		/**
		 * Shifts all the bits to the left by shift.
		 */
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

		/**
		 * Shifts all the bits to the right by shift.
		 */
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

		/**
		 * Adds one BitArray to another.
		 * Both BitArrays must have the same container, for performance considerations.
		 */
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

		/**
		 * Applies an xor between two BitArrays with the same container.
		 */
		template <u64 OtherNumBits>
		constexpr BitArray& operator^=(const BitArray<Container, OtherNumBits>& other)
		{
			apply_operator<Container, NumBits, OtherNumBits, Operator::Xor>(*this, other);
			return *this;
		}

		/**
		 * Applies an or between two BitArrays with the same container.
		 */
		template <u64 OtherNumBits>
		constexpr BitArray& operator|=(const BitArray<Container, OtherNumBits>& other)
		{
			apply_operator<Container, NumBits, OtherNumBits, Operator::Or>(*this, other);
			return *this;
		}

		/**
		 * Applies an and between two BitArrays with the same container.
		 */
		template <u64 OtherNumBits>
		constexpr BitArray& operator&=(const BitArray<Container, OtherNumBits>& other)
		{
			apply_operator<Container, NumBits, OtherNumBits, Operator::And>(*this, other);
			return *this;
		}

		/**
		 * Applies an xor between two BitArrays with different containers.
		 */
		template <typename OtherContainer, u64 OtherNumBits>
		constexpr BitArray& operator^=(const BitArray<OtherContainer, OtherNumBits>& other)
		{
			apply_operator<Container, NumBits, OtherContainer, OtherNumBits, Operator::Xor>(*this, other);
			return *this;
		}

		/**
		 * Applies an or between two BitArrays with different containers.
		 */
		template <typename OtherContainer, u64 OtherNumBits>
		constexpr BitArray& operator|=(const BitArray<OtherContainer, OtherNumBits>& other)
		{
			apply_operator<Container, NumBits, OtherContainer, OtherNumBits, Operator::Or>(*this, other);
			return *this;
		}

		/**
		 * Applies an and between two BitArrays with different containers.
		 */
		template <typename OtherContainer, u64 OtherNumBits>
		constexpr BitArray& operator&=(const BitArray<OtherContainer, OtherNumBits>& other)
		{
			apply_operator<Container, NumBits, OtherContainer, OtherNumBits, Operator::And>(*this, other);
			return *this;
		}

		/**
		 * Applies the not operator to the whole BitArray.
		 */
		constexpr BitArray& operator~()
		{
			for(u64 i = 0; i < chunk_size_; i++)
			{
				bit_chunks_[i] = ~bit_chunks_[i];
			}

			return *this;
		}

		/**
		 * Inserts obj at **CHUNK** obj_start_idx in the BitArray, assuming alignment
		 * for performance.
		 *
		 * Use only when you can assure the object's bits is a multiple of the
		 * container's bits.
		 *
		 * I.e. container is uint16_t and obj is uint32_t,
		 * while LenBits must be 32 for this case.
		 *
		 * If container type is the same as the object type, obj_start_idx can
		 * be used as an index into the objects.
		 *
		 * Note: to_bits infers this automatically
		 *
		 * @param T the type of the object to insert.
		 * @param LenBits the number of bits the obj has.
		 */
		template <typename T, u64 LenBits>
		inline constexpr void to_bits_aligned(u64 obj_start_idx, const T& obj)
		{
			// TODO make version without LenBits, inferring it from sizeof(T)
			constexpr u64 len_bytes = LenBits >> 3; // len_bits / 8
			constexpr bool is_container_sized = sizeof(Container) == sizeof(T);

			if constexpr (is_container_sized)
			{
				bit_chunks_[obj_start_idx] = *const_cast<Container*>(&obj);
				return;
			}

			u64 start_byte = (obj_start_idx * LenBits) >> 3;
			u8* start = (u8*)bit_chunks_ + start_byte;
			std::memcpy(start, &obj, len_bytes);
		}

		/**
		 * Inserts obj at bit start_idx in the BitArray, with no alignment constraints.
		 *
		 * Note: clearing is inefficient.
		 *
		 * @param T the type of the object to insert.
		 * @param Clear if the bits where the object will be inserted should be cleared first.
		 * @param LenBits the number of bits the obj has.
		 * Note that this number doesn't have to be aligned/a multiple of 8 nor the same size as T.
		 */
		template <typename T, bool Clear, u64 LenBits>
		constexpr void to_bits(u64 start_idx, const T& obj)
		{
			// TODO make version without LenBits, inferring it from sizeof(T)
			// TODO cleanup
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
					for (u64 i = 0; i < len_bytes; i++)
					{
						u16* curr = (u16*)(start + i);
						*curr |= ((u16)obj_start[i] << missaligned_bits_start);
					}

					if (extra_byte > 0)
					{
						u64 last_idx = len_bytes;
						u16 last_val = obj_start[last_idx] & overflow_mask;
						u16* curr = (u16*)(start + last_idx);
						*curr |= last_val << missaligned_bits_start;
					}
				}
			}
		}

		/**
		 * Retrieves an object at **CHUNK** obj_start_idx in the BitArray, assuming alignment
		 * for performance.
		 *
		 * Use only when you can assure the object's bits is a multiple of the
		 * container's bits.
		 *
		 * I.e. container is uint16_t and obj is uint32_t,
		 * while LenBits must be 32 for this case.
		 *
		 * If container type is the same as the object type, obj_start_idx can
		 * be used as an index into the objects.
		 *
		 * Note: from_bits infers this automatically
		 *
		 * @param T the type of the object to retrieve.
		 * @param LenBits the number of bits the obj has.
		 */
		template<typename T, u64 LenBits>
		inline constexpr T from_bits_aligned(u64 obj_start_idx) const
		{
			// TODO make version without LenBits, inferring it from sizeof(T)
			constexpr u64 len_bytes = LenBits >> 3; // len_bits / 8
			constexpr u64 true_obj_bytes = sizeof(T);
			constexpr bool is_container_sized = sizeof(Container) == sizeof(T);

			if constexpr (is_container_sized)
			{
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

		/**
		 * Retrieves an object at bit start_idx in the BitArray, with no alignment constraints.
		 *
		 * @param T the type of the object to retrieve.
		 * @param LenBits the number of bits the obj has.
		 * Note that this number doesn't have to be aligned/a multiple of 8 nor the same size as T.
		 */
		template <typename T, u64 LenBits>
		constexpr T from_bits(u64 start_idx) const
		{
			// TODO make version without LenBits, inferring it from sizeof(T)
			// TODO cleanup
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

		/**
		 * compares len bits from the two BitArrays with the same container type, starting at bit start.
		 * @return true if the two sequences match.
		 */
		constexpr bool cmp(const BitArray<Container, NumBits>& other, u64 start, u64 len) const
		{
			// TODO optimize for SIMD?

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
	};
}

#endif
