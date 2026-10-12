#ifndef FLUFF_THREADING_THREADPOOL_HPP
#define FLUFF_THREADING_THREADPOOL_HPP

#include "fluff/functional/function.hpp"
#include <condition_variable>
#include <cstdint>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>

namespace fluff::threading
{
	class ThreadPool
	{
	private:
		std::mutex queue_mutex_;
		std::condition_variable work_call_;
		std::queue<std::function<void()>> tasks_;

		std::vector<std::thread> workers_;

		bool stop_;
		void worker_loop();

		void enqueue(std::function<void()>&& task);

	public:
		ThreadPool(const ThreadPool&) = delete;
		ThreadPool(ThreadPool&&) = delete;
		ThreadPool& operator=(const ThreadPool&) = delete;
		ThreadPool& operator=(ThreadPool&&) = delete;

		ThreadPool(uint8_t threads);
		~ThreadPool();

		template<typename T>
		std::future<T> execute(Function<T()>&& func)
		{
			std::shared_ptr<std::packaged_task<T()>> task_ptr = std::make_shared<std::packaged_task<T()>>(func.get_function_raw());
			std::future<T> future = task_ptr->get_future();
			enqueue([task_ptr]() { (*task_ptr)(); });
			return future;
		}
	};
}

#endif
