
#pragma once

#include <string>
#include <cstdlib>
#include <iostream>
#include <mutex>

#include <boost/bind.hpp>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

typedef std::string (*message_callback)(const std::string&);

class session
{
public:
	session(boost::asio::io_service& io_service, message_callback mcb);

	tcp::socket& socket();

	void start();

private:
	void handle_read(const boost::system::error_code& error, std::size_t bytes_transferred);
	void handle_write(const boost::system::error_code& error);

	tcp::socket m_socket;
	static constexpr int max_length = 1024;
	char m_data[max_length];
	message_callback m_mcb;
};

class server
{
public:
	server(boost::asio::io_service& io_service, short port, message_callback mcb);

private:
	void start_accept();

	void handle_accept(session* new_session, const boost::system::error_code& error);

	boost::asio::io_service& m_io_service;
	tcp::acceptor m_acceptor;
	message_callback m_mcb;
};


