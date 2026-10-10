#ifndef FLUFF_RANDOM_MT19937_64_HPP
#define FLUFF_RANDOM_MT19937_64_HPP

#include "fluff/random/random_engine.hpp"
#include "fluff/types.hpp"
#include <random>

namespace fluff::random
{
	class Mt19937_64: public RandomEngine
	{
	private:
		std::mt19937_64 s;
		std::uniform_int_distribution<std::mt19937_64::result_type> dist;

	public:
		inline Mt19937_64(u64 seed) :
			s(seed),
			dist(0)
		{ }

		inline Mt19937_64()
		{
			std::random_device rd;
			s.seed(rd());
		}

		inline u64 next64() override
		{
			return dist(s);
		}
	};
}

#endif
