#ifndef FLUFF_FUNCTIONAL_FUNCTION_HPP
#define FLUFF_FUNCTIONAL_FUNCTION_HPP

#include "fluff/hashing/hasher.hpp"

#include <cstddef>
#include <cstdint>
#include <functional>

namespace fluff
{
	// TODO inline and/or constexpr
	template<typename Signature>
	class Function;

	template<typename T, typename... Args>
	class Function<T(Args...)>
	{
		private:
			const std::function<T(Args...)> func_;
			const std::uint64_t obj_;

		public:
			template<typename O>
			Function(T (O::*member_func)(Args...), O* obj) :
				func_([obj, member_func](Args... args) -> T { return (obj->*member_func)(std::forward<Args>(args)...); }),
				obj_((uint64_t)(void *)obj)
			{
			}

			Function(std::function<T(Args...)> func) :
				func_(func),
				obj_(0)
			{
			}

			T operator()(Args&&... args) const
			{
				return func_(std::forward<Args>(args)...);
			}

			uint64_t get_obj() const
			{
				return obj_;
			}

			std::function<T(Args...)> get_function_raw() const
			{
				return func_;
			}

			bool operator==(const Function& other) const
			{
				return (func_.target_type().name() == other.func_.target_type().name()) && (obj_ == other.obj_);
			}

			std::size_t hash() const
			{
				std::size_t seed = 0;
				fluff::Hasher::hash_combine(seed, func_.target_type().name());
				fluff::Hasher::hash_combine(seed, obj_);
				return seed;
			}
	};
}

namespace std
{
	template<typename Signature>
	struct hash<fluff::Function<Signature>>
	{
		std::size_t operator()(const fluff::Function<Signature>& func) const
		{
			return func.hash();
		}
	};
}

#endif
