#pragma once

#include "fluff/core/random/random_engine.hpp"
#include <bit>
#include <cstdint>
#include <iostream>
#include <string>

namespace fluff::random
{
	class RandomStream
	{
	private:
		RandomEngine& engine_;
		uint64_t cache_;
		uint8_t bits_left_;

		// can be statistically slightly bad
		static inline constexpr uint64_t bound(uint64_t number, uint64_t range, uint8_t num_bits)
		{
			return (uint64_t)(((__uint128_t)number * (__uint128_t)range) >> num_bits);
		}

	public:
		inline constexpr RandomStream(RandomEngine& engine) :
			engine_(engine),
			cache_(0),
			bits_left_(0)
		{ }

		inline uint64_t next64()
		{
			return engine_.next64();
		}

		inline uint64_t next64(uint64_t range)
		{
			return next64_high(range);
		}

		// calls the engine every single time
		// choose when the engine is extremely fast
		inline uint64_t next64_high(uint64_t range)
		{
			return bound(engine_.next64(), range, 64);
		}

		// calls the engine a lot
		// choose when engine is moderately fast
		inline uint64_t next64_med(uint64_t range)
		{
			const uint8_t required_bits = std::bit_width(range);
			const uint8_t leftover_bits = 64 - required_bits;

			if (required_bits > bits_left_)
			{
				cache_ = engine_.next64();
				bits_left_ = leftover_bits;
			}
			else
			{
				bits_left_ -= required_bits;
			}

			uint64_t result = (cache_ << (bits_left_ - required_bits)) >> leftover_bits;

			return bound(result, range, required_bits);
		}

		// minimizes the amount of times the engine is called
		// also maximizes the state space of the engine
		// however it has a big overhead
		// choose when engine is very slow
		uint64_t next64_low(uint64_t range)
		{
			// can be slow if the bits_left_ is a the "middle" and then a ton of 64-bit ranged calls appear
			// instead of just generating the 64 bit random number, it is also doing some shifts and assignments
			uint64_t result = 0;

			const uint8_t required_bits = std::bit_width(range);
			if (required_bits > bits_left_)
			{
				const uint8_t bits_required_next = required_bits - bits_left_;
				const uint8_t leftover_bits_next = 64 - bits_required_next;

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
