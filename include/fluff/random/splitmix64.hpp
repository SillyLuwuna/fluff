#ifndef FLUFF_RANDOM_SPLITMIX64_HPP
#define FLUFF_RANDOM_SPLITMIX64_HPP

#include "fluff/random/random_engine.hpp"
#include "fluff/types.hpp"
#include <random>

namespace fluff::random
{
	class SplitMix64 : public RandomEngine
	{
	private:
		u64 s;

	public:
		inline constexpr SplitMix64(u64 seed)
		{
			s = seed;
		}

		inline SplitMix64()
		{
			std::random_device rd;
			s = rd();
		}

		inline constexpr u64 next64() override
		{
			u64 result = (s += 0x9E3779B97F4A7C15);
			result = (result ^ (result >> 30)) * 0xBF58476D1CE4E5B9;
			result = (result ^ (result >> 27)) * 0x94D049BB133111EB;
			return result ^ (result >> 31);
		}

		inline constexpr u32 next32() override
		{
			u32 result = (s += 0x9E3779B9);
			result = result ^ (result >> 16) * 0x21F0AAAD;
			result = result ^ (result >> 15) * 0x735A2D97;
			return result ^ (result >> 15);
		}

		inline constexpr f64 nextf64() override
		{
			return (f64)next64() / (f64)std::numeric_limits<u64>::max();
		}

		inline constexpr f32 nextf32() override
		{
			return (f32)next32() / (f32)std::numeric_limits<u32>::max();
		}
	};
}

#endif
