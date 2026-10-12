#ifndef FLUFF_EVENTS_EVENT_HPP
#define FLUFF_EVENTS_EVENT_HPP

#include "fluff/functional/function.hpp"
#include <unordered_set>

namespace fluff
{
	// TODO inline and/or constexpr
	template <typename ...Args>
	using EventFunction = Function<void(Args...)>;

	template<typename ...Args>
	class Event
	{
	private:
		std::unordered_set<EventFunction<Args...>> subscribers_;

	public:
		Event() :
			subscribers_()
		{
		}

		template<typename O>
		void subscribe(void (O::*member_func)(Args...), O* obj)
		{
			subscribe(EventFunction<Args...>(member_func, obj));
		}

		template<typename O>
		void unsubscribe(void (O::*member_func)(Args...), O* obj)
		{
			unsubscribe(EventFunction<Args...>(member_func, obj));
		}

		void subscribe(std::function<void(Args...)>&& func)
		{
			subscribe(EventFunction<Args...>(func));
		}

		void unsubscribe(std::function<void(Args...)>&& func)
		{
			unsubscribe(EventFunction<Args...>(func));
		}

		virtual void subscribe(EventFunction<Args...>&& fun)
		{
			subscribers_.emplace(fun);
		}

		virtual void unsubscribe(EventFunction<Args...>&& fun)
		{
			subscribers_.erase(fun);
		}

		// does not guarantee order of execution
		virtual void invoke(Args... arg)
		{
			for (const EventFunction<Args...>& func : subscribers_)
			{
				func(std::forward<Args>(arg)...);
			}
		}

		void operator+=(EventFunction<Args...>&& other)
		{
			subscribe(std::move(other));
		}

		void operator-=(EventFunction<Args...>&& other)
		{
			unsubscribe(std::move(other));
		}

		void operator+=(std::function<void(Args...)>&& other)
		{
			subscribe(std::move(other));
		}

		void operator-=(std::function<void(Args...)>&& other)
		{
			unsubscribe(std::move(other));
		}
	};
}

#endif
