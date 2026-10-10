#ifndef FLUFF_RANDOM_RANDOM_STREAM_HPP
#define FLUFF_RANDOM_RANDOM_STREAM_HPP

#include "fluff/random/random_engine.hpp"
#include <bit>
#include "fluff/types.hpp"

namespace fluff::random
{
	class RandomStream
	{
	private:
		RandomEngine& engine_;
		u64 cache_;
		u8 bits_left_;

		// can be statistically slightly bad
		static inline constexpr u64 bound(u64 number, u64 range, u8 num_bits)
		{
			return (u64)(((u128)number * (u128)range) >> num_bits); // WARNING assumes 128-bit registers
		}

	public:
		inline constexpr RandomStream(RandomEngine& engine) :
			engine_(engine),
			cache_(0),
			bits_left_(0)
		{ }

		inline u64 next64()
		{
			return engine_.next64();
		}

		inline u64 next64(u64 range)
		{
			return next64_high(range);
		}

		// calls the engine every single time
		// choose when the engine is extremely fast
		inline u64 next64_high(u64 range)
		{
			return bound(engine_.next64(), range, 64);
		}

		// calls the engine a lot
		// choose when engine is moderately fast
		inline u64 next64_med(u64 range)
		{
			const u8 required_bits = std::bit_width(range);
			const u8 leftover_bits = 64 - required_bits;

			if (required_bits > bits_left_)
			{
				cache_ = engine_.next64();
				bits_left_ = leftover_bits;
			}
			else
			{
				bits_left_ -= required_bits;
			}

			u64 result = (cache_ << (bits_left_ - required_bits)) >> leftover_bits;

			return bound(result, range, required_bits);
		}

		// minimizes the amount of times the engine is called
		// also maximizes the state space of the engine
		// however it has a big overhead
		// choose when engine is very slow
		u64 next64_low(u64 range)
		{
			// can be slow if the bits_left_ is a the "middle" and then a ton of 64-bit ranged calls appear
			// instead of just generating the 64 bit random number, it is also doing some shifts and assignments
			u64 result = 0;

			const u8 required_bits = std::bit_width(range);
			if (required_bits > bits_left_)
			{
				const u8 bits_required_next = required_bits - bits_left_;
				const u8 leftover_bits_next = 64 - bits_required_next;

				if (bits_left_ != 0)
				{
					result |= (cache_ >> (64 - bits_left_));
				}

				cache_ = engine_.next64();

				result |= (cache_ << leftover_bits_next) >> (leftover_bits_next - bits_left_);
				bits_left_ = leftover_bits_next;
			}
			else
			{
				result |= (cache_ << (bits_left_ - required_bits)) >> (64 - required_bits);
				bits_left_ -= required_bits;
			}

			return bound(result, range, required_bits);
		}
	};
}

#endif
