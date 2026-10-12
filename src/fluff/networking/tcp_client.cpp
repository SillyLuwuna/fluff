#include "fluff/networking/tcp_client.hpp"
#include "fluff/events/event.hpp"

#include <event2/thread.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <netinet/tcp.h> // linux only?
#include <stdexcept>
#include <thread>

using namespace fluff::networking;

TcpClient::TcpClient() :
	base_(nullptr),
	event_buffer_(nullptr),
	connected_(false),
	event_invoker_(1) // MUST be 1
{
	evthread_use_pthreads();

	base_ = event_base_new();
	if (!base_)
	{
		throw std::runtime_error("failed to create event base");
	}
}

TcpClient::~TcpClient()
{
	disconnect();

	if (event_thread_.joinable())
	{
		event_thread_.join();
	}

	if (base_)
	{
		event_base_free(base_);
	}
}

// returns if an error ocurred or not
bool TcpClient::connect(const std::string& ip, uint16_t port)
{
	std::lock_guard lock(is_connecting_or_disconnecting_mutex_);

	if (connected_.load()) return true;

	std::lock_guard lock_buffer(event_buffer_mutex_);

	evutil_socket_t sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0)
	{
		std::cerr << "failed to create socket\n";
		return false;
	}

	evutil_make_socket_nonblocking(sock);

	int flag_set = 1;
	int socketop_success = setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, &flag_set, sizeof(flag_set));
	if (!socketop_success)
	{
		std::cerr << "could not set socket options\n";
	}

	hostent* host = gethostbyname(ip.c_str());
	if (!host)
	{
		std::cerr << "failed to resolve hostname: " << host << "\n";
		evutil_closesocket(sock);
		return false;
	}

	sockaddr_in sin;
	std::memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(port);
	std::memcpy(&sin.sin_addr, host->h_addr_list[0], host->h_length);

	event_buffer_ = bufferevent_socket_new(base_, sock, BEV_OPT_CLOSE_ON_FREE);
	if (!event_buffer_)
	{
		std::cerr << "failed to create buffer event\n";
		evutil_closesocket(sock);
		return false;
	}

	bufferevent_setcb(event_buffer_, on_read, nullptr, on_event, this);
	bufferevent_enable(event_buffer_, EV_READ | EV_WRITE);

	int success = bufferevent_socket_connect(event_buffer_, (sockaddr*)&sin, sizeof(sin));
	if (success < 0)
	{
		std::cerr << "failed to connect\n";
		bufferevent_free(event_buffer_);
		event_buffer_ = nullptr;
		return false;
	}

	connect_result_done_ = false;

	if (event_thread_.joinable())
	{
		event_thread_.join();
	}
	event_thread_ = std::jthread(&TcpClient::run, this); // todo make something more efficient than always starting a new thread

	std::unique_lock connecting_lock(connecting_);
	connect_result_condition_.wait(connecting_lock, [this]() { return this->connect_result_done_; });
	connecting_lock.unlock();

	if (!connect_result_)
	{
		event_base_loopbreak(base_);

		if (event_buffer_)
		{
			bufferevent_free(event_buffer_);
			event_buffer_ = nullptr;
		}

		return false;
	}

	connected_.store(true);
	event_invoker_.execute(Function<void()>(&ThreadSafeEvent<>::invoke, &connect_event_));

	return true;
}

void TcpClient::disconnect()
{
	std::lock_guard lock(is_connecting_or_disconnecting_mutex_);

	if (!connected_.exchange(false)) return;

	event_base_loopbreak(base_);

	{
		std::lock_guard lock(event_buffer_mutex_);
		if (event_buffer_)
		{
			bufferevent_free(event_buffer_);
			event_buffer_ = nullptr;
		}
	}

	event_invoker_.execute(Function<void()>(&ThreadSafeEvent<>::invoke, &disconnect_event_));
}

bool TcpClient::is_worker_thread()
{
	return std::this_thread::get_id() == event_thread_.get_id();
}

bool TcpClient::is_connected() const
{
	return connected_.load();
}

void TcpClient::run()
{
	int success = event_base_dispatch(base_);
	if (success < 0)
	{
		std::cerr << "error on stopping event loop\n";
	}
	else if (success > 0)
	{
		// std::cerr << "out of events in loop\n";
	}
}

void TcpClient::send_data(const std::vector<uint8_t>& data)
{
	send_data(data.data(), data.size());
}

void TcpClient::send_data(const uint8_t* data, size_t size)
{
	std::lock_guard lock(event_buffer_mutex_);

	if (!connected_.load()) return;
	if (!event_buffer_) return;

	int success = bufferevent_write(event_buffer_, data, size);
	if (success < 0)
	{
		std::cerr << "could not send message\n";
	}
}

void TcpClient::on_read(bufferevent* buffer, void* ctx)
{
	TcpClient* client = static_cast<TcpClient*>(ctx);
	evbuffer* input = bufferevent_get_input(buffer);

	size_t len = evbuffer_get_length(input);
	if (len == 0) return;

	std::vector<uint8_t> data(len);
	evbuffer_remove(input, data.data(), len);

	client->event_invoker_.execute(Function<void()>(
		[d = std::move(data), client]() { client->data_received_event_.invoke(d); }
	));
}

void TcpClient::on_event(bufferevent*, short events, void* ctx)
{
	TcpClient* client = static_cast<TcpClient*>(ctx);

	if (events & BEV_EVENT_CONNECTED)
	{
		// std::cout << "connected to server\n";
		if (!client->connected_.load())
		{
			client->connect_result_ = true;
			client->connect_result_done_ = true;
			client->connect_result_condition_.notify_one();
		}
	}

	if (events & BEV_EVENT_ERROR)
	{
		std::cerr << "socket error: " << evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()) << "\n";
	}

	if (events & (BEV_EVENT_EOF | BEV_EVENT_ERROR))
	{
		if (!client->connected_.load())
		{
			client->connect_result_ = false;
			client->connect_result_done_ = true;
			client->connect_result_condition_.notify_one();
		}
		else
		{
			// std::cout << "disconnected from server\n";
			client->disconnect();
		}
	}
}

fluff::Event<>& TcpClient::connection_event()
{
	return connect_event_;
}

fluff::Event<>& TcpClient::disconnection_event()
{
	return disconnect_event_;
}

fluff::Event<const std::vector<uint8_t>&>& TcpClient::data_received_event()
{
	return data_received_event_;
}
