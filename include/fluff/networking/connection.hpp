#ifndef FLUFF_NETWORKING_CONNECTION_HPP
#define FLUFF_NETWORKING_CONNECTION_HPP

#include <string>
#include <event2/bufferevent.h>
#include "fluff/types.hpp"

namespace fluff::networking
{
	using namespace fluff::types;

	struct Connection
	{
		std::string ip;
		u16 port;
		std::string endpoint;
		struct bufferevent* buffer_event;
		i32 fd;

		Connection(const std::string& ip, u16 port, struct bufferevent* buffer_event);

		std::size_t hash() const;
		bool operator==(const Connection& other) const;

	private:
		static std::string get_endpoint(std::string ip, u16 port);
	};
}

namespace std
{
	template<>
	struct hash<fluff::networking::Connection>
	{
		std::size_t operator()(const fluff::networking::Connection& client) const
		{
			return client.hash();
		}
	};
}

#endif
