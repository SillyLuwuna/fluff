#pragma once

#include "random/random_engine.hpp"
#include <cstdint>
#include <random>

namespace fluff::random
{
	class SplitMix64 : public RandomEngine
	{
	private:
		uint64_t s;

	public:
		inline constexpr SplitMix64(uint64_t seed)
		{
			s = seed;
		}

		inline SplitMix64()
		{
			std::random_device rd;
			s = rd();
		}

		inline constexpr uint64_t next64() override
		{
			uint64_t result = (s += 0x9E3779B97F4A7C15);
			result = (result ^ (result >> 30)) * 0xBF58476D1CE4E5B9;
			result = (result ^ (result >> 27)) * 0x94D049BB133111EB;
			return result ^ (result >> 31);
		}

		inline constexpr uint32_t next32() override
		{
			uint32_t result = (s += 0x9E3779B9);
			result = result ^ (result >> 16) * 0x21F0AAAD;
			result = result ^ (result >> 15) * 0x735A2D97;
			return result ^ (result >> 15);
		}

		inline constexpr double nextf64() override
		{
			return (double)next64() / (double)std::numeric_limits<uint64_t>::max();
		}

		inline constexpr float nextf32() override
		{
			return (float)next32() / (float)std::numeric_limits<uint32_t>::max();
		}
	};
}
