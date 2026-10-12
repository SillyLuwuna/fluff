#ifndef FLUFF_NETWORKING_TCP_SERVER_HPP
#define FLUFF_NETWORKING_TCP_SERVER_HPP

#include <cstdint>
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

namespace fluff::networking
{
	class TcpServer
	{
	private:
		uint16_t max_connections_;
		uint16_t port_;

		ThreadSafeEvent<> start_event_;
		ThreadSafeEvent<> stop_event_;
		ThreadSafeEvent<const Connection&> connection_event_;
		ThreadSafeEvent<const Connection&> disconnection_event_;
		ThreadSafeEvent<const Connection&, const std::vector<uint8_t>&> data_received_event_;

		event_base* base_;
		evconnlistener* listener_;
		std::unordered_map<int, Connection> connection_fds_;
		std::unordered_set<Connection> connections_;
		mutable std::mutex connections_mutex_;

		std::mutex is_connecting_or_disconnecting_mutex_;
		std::atomic<bool> running_;

		std::jthread event_thread_;
		fluff::threading::ThreadPool event_invoker_;

		void run();

		static void on_accept(evconnlistener* listener, evutil_socket_t fd, sockaddr* address, int socklen, void* ctx);
		static void on_read(bufferevent* buffer, void* ctx);
		static void on_event(bufferevent* buffer, short events, void* ctx);

		void disconnect(int fd);

	public:
		TcpServer(const TcpServer&) = delete;
		TcpServer(TcpServer&&) = delete;
		TcpServer& operator=(const TcpServer&) = delete;
		TcpServer& operator=(TcpServer&&) = delete;

		TcpServer(uint16_t max_connections, uint16_t port);
		~TcpServer();

		bool start();
		void stop();

		bool is_connected(const Connection& connection) const;
		void disconnect(Connection& connection);
		void send_data(const std::vector<uint8_t>& data, const Connection& connection);
		void send_data(const uint8_t* data, size_t size, const Connection& connection);

		std::vector<const Connection*> connections() const;

		uint16_t port();
		uint16_t max_connections();

		Event<>& start_event();
		Event<>& stop_event();
		Event<const Connection&>& connection_event();
		Event<const Connection&>& disconnection_event();
		Event<const Connection&, const std::vector<uint8_t>&>& data_received_event();
	};
}

#endif
