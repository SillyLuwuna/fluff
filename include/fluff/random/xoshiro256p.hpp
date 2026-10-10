#ifndef FLUFF_RANDOM_XOSHIRO256P_HPP
#define FLUFF_RANDOM_XOSHIRO256P_HPP

#include "fluff/random/splitmix64.hpp"
#include "fluff/random/random_engine.hpp"
#include "fluff/types.hpp"

namespace fluff::random
{

	// bad lower bits (+ version)
	// better for 32bit floating point generation
	// TODO parallelizable
	// TODO jump function for parallelism
	// TODO better statistical abilities
	class Xoshiro256p : public RandomEngine
	{
	private:
		u64 s[4];

		static inline constexpr u64 rol64(u64 x, i32 k)
		{
			return (x << k) | (x >> (64 - k));
		}

	public:
		inline constexpr Xoshiro256p(u64 seed)
		{
			SplitMix64 sm64(seed);

			s[0] = sm64.next64();
			s[1] = sm64.next64();
			s[2] = sm64.next64();
			s[3] = sm64.next64();
		}

		inline Xoshiro256p()
		{
			SplitMix64 sm64;

			s[0] = sm64.next64();
			s[1] = sm64.next64();
			s[2] = sm64.next64();
			s[3] = sm64.next64();
		}

		inline constexpr u64 next64() override
		{
			const u64 result = s[0] + s[3];
			const u64 t = s[1] << 17;

			s[2] ^= s[0];
			s[3] ^= s[1];
			s[1] ^= s[2];
			s[0] ^= s[3];

			s[2] ^= t;
			s[3] = rol64(s[3], 45);

			return result;
		}

		inline constexpr u32 next32() override
		{
			return (u32)(next64() >> 32);
		}

		inline constexpr f64 nextf64() override
		{
			return (f64)next64() / (f64)std::numeric_limits<u64>::max();
		}

		inline constexpr f32 nextf32() override
		{
			return (f32)(next64() >> 32) / (f32)std::numeric_limits<u32>::max();
		}
	};
}

#endif
