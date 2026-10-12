#ifndef FLUFF_EVENTS_THREAD_SAFE_EVENT_HPP
#define FLUFF_EVENTS_THREAD_SAFE_EVENT_HPP

#include "fluff/events/event.hpp"

#include <atomic>
#include <mutex>
#include <queue>

namespace fluff
{
	template<typename ...Args>
	class ThreadSafeEvent : public Event<Args...>
	{
	private:
		std::mutex event_mutex_;

		std::atomic<bool> invoked_;
		std::mutex queue_mutex_;
		std::queue<std::pair<EventFunction<Args...>, bool>> subscription_queue_;

		void dequeue_all()
		{
			std::lock_guard lock(queue_mutex_);
			while (subscription_queue_.size() > 0)
			{
				std::pair<EventFunction<Args...>, bool> subscription = subscription_queue_.front();

				if (subscription.second)
				{
					Event<Args...>::subscribe(std::move(subscription.first));
				}
				else
				{
					Event<Args...>::unsubscribe(std::move(subscription.first));
				}

				subscription_queue_.pop();
			}
		}

		void enqueue_subscription(EventFunction<Args...>&& fun)
		{
			std::lock_guard queue_lock(queue_mutex_);
			subscription_queue_.emplace(std::pair<EventFunction<Args...>, bool>(std::move(fun), true));
		}

		void enqueue_unsubscription(EventFunction<Args...>&& fun)
		{
			std::lock_guard queue_lock(queue_mutex_);
			subscription_queue_.emplace(std::pair<EventFunction<Args...>, bool>(std::move(fun), false));
		}

	public:
		ThreadSafeEvent() :
			Event<Args...>(),
			event_mutex_(),
			invoked_(false),
			queue_mutex_(),
			subscription_queue_()
		{
		}

		void subscribe(EventFunction<Args...>&& fun) override
		{
			if (invoked_.load())
			{
				enqueue_subscription(std::move(fun));
				return;
			}

			std::lock_guard lock(event_mutex_);

			dequeue_all();
			Event<Args...>::subscribe(std::move(fun));
		}

		virtual void unsubscribe(EventFunction<Args...>&& fun) override
		{
			if (invoked_.load())
			{
				enqueue_unsubscription(std::move(fun));
				return;
			}

			std::lock_guard lock(event_mutex_);

			dequeue_all();
			Event<Args...>::unsubscribe(std::move(fun));
		}

		// does not guarantee order of execution
		virtual void invoke(Args... arg) override
		{
			invoked_.store(true);
			std::lock_guard lock(event_mutex_);

			dequeue_all();
			Event<Args...>::invoke(std::forward<Args>(arg)...);

			invoked_.store(false);
		}
	};
}

#endif
