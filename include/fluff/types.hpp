#ifndef FLUFF_TYPES_HPP
#define FLUFF_TYPES_HPP

#include <cstdint>
#include <stdfloat>

namespace fluff {
	using u8 = std::uint8_t;
	using u16 = std::uint16_t;
	using u32 = std::uint32_t;
	using u64 = std::uint64_t;
	using u128 = unsigned __int128; // WARNING assumes 128-bit registers

	using i8 = std::int8_t;
	using i16 = std::int16_t;
	using i32 = std::int32_t;
	using i64 = std::int64_t;
	using i128 = __int128; // WARNING assumes 128-bit registers

#if defined(__STDCPP_FLOAT16_T__)
	using f16 = std::float16_t;
#else
	using f16 = float;
#endif

#if defined(__STDCPP_FLOAT16_T__)
	using f32 = std::float32_t;
#else
	using f32 = float;
#endif

#if defined(__STDCPP_FLOAT16_T__)
	using f64 = std::float64_t;
#else
	using f64 = double;
#endif
}

#endif
