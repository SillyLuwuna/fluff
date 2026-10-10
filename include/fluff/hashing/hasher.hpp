#ifndef FLUFF_HASHING_HASHER_HPP
#define FLUFF_HASHING_HASHER_HPP

#include <cstddef>
#include <functional>

namespace fluff
{
	struct Hasher
	{
		template <class T>
		static inline constexpr void hash_combine(std::size_t& seed, const T& v)
		{
			seed ^= std::hash<T>{}(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
		}
	};
}


#endif
