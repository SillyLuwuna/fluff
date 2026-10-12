#ifndef FLUFF_NETWORKING_TCP_CLIENT_HPP
#define FLUFF_NETWORKING_TCP_CLIENT_HPP

#include <mutex>
#include <string>
#include <thread>
#include <event2/event.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>

#include "fluff/events/event.hpp"
#include "fluff/events/thread_safe_event.hpp"
#include "fluff/threading/threadpool.hpp"

namespace fluff::networking
{
	// TODO instead of creating a new thread all the time keep the same thread working and activate it / deactivate it
	using namespace fluff::types;

	class TcpClient
	{
	private:
		ThreadSafeEvent<> connect_event_;
		ThreadSafeEvent<> disconnect_event_;
		ThreadSafeEvent<const std::vector<u8>&> data_received_event_;

		event_base* base_;
		bufferevent* event_buffer_;
		std::mutex event_buffer_mutex_;

		std::atomic<bool> connected_;
		std::mutex is_connecting_or_disconnecting_mutex_;

		std::jthread event_thread_;
		fluff::threading::ThreadPool event_invoker_;

		std::mutex connecting_;
		std::condition_variable connect_result_condition_;
		bool connect_result_done_;
		bool connect_result_;

		void run();
		bool is_worker_thread();

		static void on_read(bufferevent* buffer, void* ctx);
		static void on_event(bufferevent* buffer, short events, void* ctx);

	public:
		TcpClient(const TcpClient&) = delete;
		TcpClient(TcpClient&&) = delete;
		TcpClient& operator=(const TcpClient&) = delete;
		TcpClient& operator=(TcpClient&&) = delete;

		TcpClient();
		~TcpClient();

		bool connect(const std::string& ip, u16 port);
		void disconnect();

		bool is_connected() const;

		void send_data(const std::vector<u8>& data);
		void send_data(const u8* data, size_t size);

		Event<>& connection_event();
		Event<>& disconnection_event();
		Event<const std::vector<u8>&>& data_received_event();
	};
}

#endif
