#ifndef FLUFF_BENCHMARK_TIMER_HPP
#define FLUFF_BENCHMARK_TIMER_HPP

#include "fluff/types.hpp"
#include <chrono>

namespace fluff::benchmark
{
	using namespace fluff::types;

	/**
	 * Global timer class.
	 * Note: WIP, should not be global
	 */
	class Timer
	{
		// TODO confusing. fix. Should not be global

	private:
		static inline std::chrono::time_point<std::chrono::steady_clock, std::chrono::duration<long, std::ratio<1, 1000000000>>> start_time_;

	public:

		/**
		 * Starts the clock.
		 */
		static inline void start()
		{
			start_time_ = std::chrono::steady_clock::now();
		}

		/**
		 * Restarts the clock.
		 */
		static inline void restart()
		{
			start_time_ = std::chrono::steady_clock::now();
		}

		/**
		 * Stops the clock.
		 */
		static inline void stop()
		{
			elapsed_ns();
		}

		/**
		 * Get the elapsed time since either starting the clock, in nanoseconds.
		 */
		static inline u64 elapsed_ns()
		{
			return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - start_time_).count();
		}

		/**
		 * Get the elapsed time since either starting the clock, in milliseconds.
		 */
		static inline u64 elapsed_ms()
		{
			return elapsed_ns() / 1000000;
		}
	};
}

#endif
