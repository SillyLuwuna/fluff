#include "fluff/networking/connection.hpp"

using fluff::networking::Connection;

Connection::Connection(const std::string& ip, u16 port, struct bufferevent* buffer_event) :
	ip(ip),
	port(port),
	endpoint(get_endpoint(ip, port)),
	buffer_event(buffer_event),
	fd(bufferevent_getfd(buffer_event))
{
}

std::size_t Connection::hash() const
{
	return std::hash<std::string>{}(endpoint);
}

bool Connection::operator==(const Connection& other) const
{
	return this->endpoint == other.endpoint;
}

std::string Connection::get_endpoint(std::string ip, u16 port)
{
	return ip + ":" + std::to_string(port);
}

