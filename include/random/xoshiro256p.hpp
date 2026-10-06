#pragma once

#include "random/splitmix64.hpp"
#include "random/random_engine.hpp"
#include <cstdint>
#include <random>

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
		uint64_t s[4];

		static inline constexpr uint64_t rol64(uint64_t x, int32_t k)
		{
			return (x << k) | (x >> (64 - k));
		}

	public:
		inline constexpr Xoshiro256p(uint64_t seed)
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

		inline constexpr uint64_t next64() override
		{
			const uint64_t result = s[0] + s[3];
			const uint64_t t = s[1] << 17;

			s[2] ^= s[0];
			s[3] ^= s[1];
			s[1] ^= s[2];
			s[0] ^= s[3];

			s[2] ^= t;
			s[3] = rol64(s[3], 45);

			return result;
		}

		inline constexpr uint32_t next32() override
		{
			return (uint32_t)(next64() >> 32);
		}

		inline constexpr double nextf64() override
		{
			return (double)next64() / (double)std::numeric_limits<uint64_t>::max();
		}

		inline constexpr float nextf32() override
		{
			return (float)(next64() >> 32) / (float)std::numeric_limits<uint32_t>::max();
		}
	};
}
