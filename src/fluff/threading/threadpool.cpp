#include <arpa/inet.h>
#include "fluff/threading/threadpool.hpp"

using namespace fluff::threading;

ThreadPool::ThreadPool(u8 threads) :
	stop_(false)
{
	workers_.reserve(threads);
	for (u8 i = 0; i < threads; i++)
	{
		workers_.emplace_back(&ThreadPool::worker_loop, this);
	}
}

ThreadPool::~ThreadPool()
{
	stop_ = true;
	work_call_.notify_all();

	for (std::thread& worker : workers_)
	{
		worker.join();
	}
}

void ThreadPool::worker_loop()
{
	while (true)
	{
		std::function<void()> task;

		{
			std::unique_lock lock(queue_mutex_);
			work_call_.wait(lock, [this]() { return !tasks_.empty() || stop_; });

			if (stop_ && tasks_.empty()) return;

			task = std::move(tasks_.front());
			tasks_.pop();
		}

		task();
	}
}

void ThreadPool::enqueue(std::function<void()>&& task)
{
	{
		std::lock_guard lock(queue_mutex_);
		tasks_.emplace(std::move(task));
	}
	work_call_.notify_one();
}
