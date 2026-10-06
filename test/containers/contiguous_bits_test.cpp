#include "doctest.h"
#include "containers/contiguous_bits.hpp"

using turing_learning::containers::ContiguousBits;

constexpr uint64_t fast_pow(uint64_t base, uint64_t exponent)
{
	uint64_t result = base;

	for (uint64_t i = 0; i < exponent - 1; i++)
	{
		result *= base;
	}

	return result;
}

TEST_CASE("should contain aligned items")
{
	const uint64_t amount = 64;
	const uint64_t num_bits = 16;

	std::srand(137);
	std::vector<uint16_t> saved_bits(amount);
	ContiguousBits<uint16_t, uint16_t, num_bits, amount> bits;

	for (uint64_t i = 0; i < amount; i++)
	{
		const uint64_t max_val = fast_pow(2, num_bits);
		uint16_t random_value = (std::rand() % max_val);

		bits.rewrite_at(i, random_value);
		saved_bits[i] = random_value;
	}

	for (uint64_t i = 0; i < amount; i++)
	{
		CHECK(bits[i] == saved_bits[i]);
	}
}

TEST_CASE("should contain shorter misaligned items")
{
	const uint64_t amount = 64;
	const uint64_t num_bits = 5;

	std::srand(137);
	std::vector<uint8_t> saved_bits(amount);
	ContiguousBits<uint8_t, uint8_t, num_bits, amount> bits;

	for (uint64_t i = 0; i < amount; i++)
	{
		const uint64_t max_val = fast_pow(2, num_bits);
		uint8_t random_value = (std::rand() % max_val);

		bits.rewrite_at(i, random_value);
		saved_bits[i] = random_value;
	}

	for (uint64_t i = 0; i < amount; i++)
	{
		CHECK(bits[i] == saved_bits[i]);
	}
}

TEST_CASE("should contain longer misaligned items")
{
	const uint64_t amount = 64;
	const uint64_t num_bits = 23;

	std::srand(137);
	std::vector<uint32_t> saved_bits(amount);
	ContiguousBits<uint32_t, uint8_t, num_bits, amount> bits;

	for (uint64_t i = 0; i < amount; i++)
	{
		const uint64_t max_val = fast_pow(2, num_bits);
		uint32_t random_value = (std::rand() % max_val);

		bits.rewrite_at(i, random_value);
		saved_bits[i] = random_value;
	}

	for (uint64_t i = 0; i < amount; i++)
	{
		CHECK(bits[i] == saved_bits[i]);
	}
}

TEST_CASE("should contain very large misaligned items, casting to lower")
{
	const uint64_t amount = 64;
	const uint64_t num_bits = 512;

	std::srand(137);
	std::vector<uint32_t> saved_bits(amount);
	ContiguousBits<uint32_t, uint8_t, 32, amount> bits;

	for (uint64_t i = 0; i < amount; i++)
	{
		const uint64_t max_val = fast_pow(2, 31);
		uint32_t random_value = (std::rand() % max_val);

		bits.rewrite_at(i, random_value);
		saved_bits[i] = random_value;
	}

	for (uint64_t i = 0; i < amount; i++)
	{
		CHECK(bits[i] == saved_bits[i]);
	}
}

TEST_CASE("should contain very large misaligned items")
{
	const uint64_t amount = 64;
	const uint64_t num_bits = 512;

	const uint64_t numbers = (num_bits / 8) + (num_bits % 8);
	struct big_number
	{
		uint8_t number[numbers];
	};

	std::srand(137);
	std::vector<big_number> saved_bits(amount);
	ContiguousBits<struct big_number, uint8_t, num_bits, amount> bits;

	for (uint64_t i = 0; i < amount; i++)
	{
		const uint64_t max_val = fast_pow(2, 8);

		struct big_number curr;
		for (uint64_t j = 0; j < numbers; j++)
		{
			curr.number[j] = std::rand() % max_val;
		}

		bits.rewrite_at(i, curr);
		saved_bits[i] = curr;
	}

	for (uint64_t i = 0; i < amount; i++)
	{
		for (uint64_t j = 0; j < numbers; j++)
		{
			CHECK(bits[i].number[j] == saved_bits[i].number[j]);
		}
	}
}

TEST_CASE("bigger container test")
{
	const uint64_t amount = 64;
	const uint64_t num_bits = 9;

	std::srand(137);
	std::vector<uint16_t> saved_bits(amount);
	ContiguousBits<uint16_t, uint32_t, num_bits, amount> bits;

	for (uint64_t i = 0; i < amount; i++)
	{
		const uint64_t max_val = fast_pow(2, num_bits);
		uint16_t random_value = (std::rand() % max_val);

		bits.rewrite_at(i, random_value);
		saved_bits[i] = random_value;
	}

	for (uint64_t i = 0; i < amount; i++)
	{
		CHECK(bits[i] == saved_bits[i]);
	}
}

TEST_CASE("should equals")
{
	const uint64_t amount = 64;
	const uint64_t num_bits = 9;

	std::srand(137);
	std::vector<uint16_t> saved_bits(amount);
	ContiguousBits<uint16_t, uint32_t, num_bits, amount> bits;

	for (uint64_t i = 0; i < amount; i++)
	{
		const uint64_t max_val = fast_pow(2, num_bits);
		uint16_t random_value = (std::rand() % max_val);

		bits.rewrite_at(i, random_value);
		saved_bits[i] = random_value;
	}

	ContiguousBits<uint16_t, uint32_t, num_bits, amount> copy = bits;

	for (uint64_t i = 0; i < amount; i++)
	{
		uint16_t start = (std::rand() % amount);
		uint16_t len = (std::rand() % amount) % (amount - start);
		CHECK(bits.cmp(copy, start, len));
	}
}

TEST_CASE("should equals small items")
{
	const uint64_t amount = 64;
	const uint64_t num_bits = 3;

	std::srand(137);
	std::vector<uint16_t> saved_bits(amount);
	ContiguousBits<uint16_t, uint8_t, num_bits, amount> bits;

	for (uint64_t i = 0; i < amount; i++)
	{
		const uint64_t max_val = fast_pow(2, num_bits);
		uint16_t random_value = (std::rand() % max_val);

		bits.rewrite_at(i, random_value);
		saved_bits[i] = random_value;
	}

	ContiguousBits<uint16_t, uint8_t, num_bits, amount> copy = bits;

	for (uint64_t i = 0; i < amount; i++)
	{
		uint16_t start = (std::rand() % amount);
		uint16_t len = (std::rand() % amount) % (amount - start);
		CHECK(bits.cmp(copy, start, len));
	}
}

TEST_CASE("should not equals")
{
	const uint64_t amount = 64;
	const uint64_t num_bits = 9;

	std::srand(137);
	std::vector<uint16_t> saved_bits(amount);
	ContiguousBits<uint16_t, uint8_t, num_bits, amount> bits;

	for (uint64_t i = 0; i < amount; i++)
	{
		const uint64_t max_val = fast_pow(2, num_bits);
		uint16_t random_value = (std::rand() % max_val);

		bits.rewrite_at(i, random_value);
		saved_bits[i] = random_value;
	}

	ContiguousBits<uint16_t, uint8_t, num_bits, amount> copy = bits;
	uint64_t diff_idx = amount / 2;
	copy.rewrite_at(diff_idx, 0);

	for (uint64_t i = 0; i < amount; i++)
	{
		uint16_t start = (std::rand() % amount);
		uint16_t len = (std::rand() % amount) % (amount - start);
		if (diff_idx >= start && diff_idx < (start + len))
		{
			CHECK(!bits.cmp(copy, start, len));
		}
		else
		{
			CHECK(bits.cmp(copy, start, len));
		}
	}
}
