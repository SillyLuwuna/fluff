#pragma once

#include "random/random_engine.hpp"
#include <cstdint>
#include <random>

namespace fluff::random
{
	class Mt19937_64: public RandomEngine
	{
	private:
		std::mt19937_64 s;
		std::uniform_int_distribution<std::mt19937_64::result_type> dist;

	public:
		inline Mt19937_64(uint64_t seed) :
			s(seed),
			dist(0)
		{ }

		inline Mt19937_64()
		{
			std::random_device rd;
			s.seed(rd());
		}

		inline uint64_t next64() override
		{
			return dist(s);
		}
	};
}
