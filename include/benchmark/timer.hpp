#pragma once

#include <cstdint>
#include <chrono>

namespace fluff::benchmark
{
	class Timer
	{
	private:
		static inline std::chrono::time_point<std::chrono::steady_clock, std::chrono::duration<long, std::ratio<1, 1000000000>>> start_time_;
		// static std::chrono::time_point<std::chrono::steady_clock, std::chrono::duration<long, std::ratio<1, 1000000000>>> end_time;

		static inline uint64_t total_elapsed_ns_;

	public:
		static inline void restart()
		{
			total_elapsed_ns_ = 0;
		}

		static inline void start()
		{
			start_time_ = std::chrono::steady_clock::now();
		}

		static inline void stop()
		{
			elapsed_ns();
		}

		static inline uint64_t elapsed_ns()
		{
			uint64_t curr_elapsed_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - start_time_).count();
			total_elapsed_ns_ += curr_elapsed_ns;
			return curr_elapsed_ns;
		}

		static inline uint64_t elapsed_ms()
		{
			return elapsed_ns() / 1000000;
		}

		static inline uint64_t total_elapsed_ms()
		{
			return total_elapsed_ns_ / 1000000;
		}
	};
}
