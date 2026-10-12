#ifndef FLUFF_NETWORKING_TCP_SERVER_HPP
#define FLUFF_NETWORKING_TCP_SERVER_HPP

#include <mutex>
#include <thread>
#include <event2/event.h>
#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>

#include "fluff/events/event.hpp"
#include "fluff/events/thread_safe_event.hpp"
#include "fluff/networking/connection.hpp"
#include "fluff/threading/threadpool.hpp"
#include "fluff/types.hpp"

namespace fluff::networking
{
	using namespace fluff::types;

	class TcpServer
	{
	private:
		u16 max_connections_;
		u16 port_;

		ThreadSafeEvent<> start_event_;
		ThreadSafeEvent<> stop_event_;
		ThreadSafeEvent<const Connection&> connection_event_;
		ThreadSafeEvent<const Connection&> disconnection_event_;
		ThreadSafeEvent<const Connection&, const std::vector<u8>&> data_received_event_;

		event_base* base_;
		evconnlistener* listener_;
		std::unordered_map<i32, Connection> connection_fds_;
		std::unordered_set<Connection> connections_;
		mutable std::mutex connections_mutex_;

		std::mutex is_connecting_or_disconnecting_mutex_;
		std::atomic<bool> running_;

		std::jthread event_thread_;
		fluff::threading::ThreadPool event_invoker_;

		void run();

		static void on_accept(evconnlistener* listener, evutil_socket_t fd, sockaddr* address, i32 socklen, void* ctx);
		static void on_read(bufferevent* buffer, void* ctx);
		static void on_event(bufferevent* buffer, short events, void* ctx);

		void disconnect(i32 fd);

	public:
		TcpServer(const TcpServer&) = delete;
		TcpServer(TcpServer&&) = delete;
		TcpServer& operator=(const TcpServer&) = delete;
		TcpServer& operator=(TcpServer&&) = delete;

		TcpServer(u16 max_connections, u16 port);
		~TcpServer();

		bool start();
		void stop();

		bool is_connected(const Connection& connection) const;
		void disconnect(Connection& connection);
		void send_data(const std::vector<u8>& data, const Connection& connection);
		void send_data(const u8* data, size_t size, const Connection& connection);

		std::vector<const Connection*> connections() const;

		u16 port();
		u16 max_connections();

		Event<>& start_event();
		Event<>& stop_event();
		Event<const Connection&>& connection_event();
		Event<const Connection&>& disconnection_event();
		Event<const Connection&, const std::vector<u8>&>& data_received_event();
	};
}

#endif
