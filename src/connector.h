
#pragma once

#include <string>
#include <cstdlib>
#include <iostream>

#include <boost/bind.hpp>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

typedef std::string (*message_callback)(const std::string&);

class session
{
public:
	session(boost::asio::io_service& io_service, message_callback mcb)
		: m_socket(io_service)
		, m_mcb(mcb)
	{
	}

	tcp::socket& socket()
	{
		return m_socket;
	}

	void start()
	{
		m_socket.async_read_some(boost::asio::buffer(m_data, max_length),
			boost::bind(&session::handle_read, this,
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));
	}

private:
	void handle_read(const boost::system::error_code& error, std::size_t bytes_transferred)
	{
		if (!error)
		{
			std::string msg(m_data, m_data+bytes_transferred);
			std::string ret = m_mcb(msg);
			boost::asio::async_write(m_socket,
				boost::asio::buffer(ret.c_str(), ret.size()),
				boost::bind(&session::handle_write, this,
				boost::asio::placeholders::error));
		} else {
			delete this;
		}
	}

	void handle_write(const boost::system::error_code& error)
	{
		if (!error)
		{
			m_socket.async_read_some(boost::asio::buffer(m_data, max_length),
				boost::bind(&session::handle_read, this,
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred));
		} else {
		  delete this;
		}
	}

	tcp::socket m_socket;
	static constexpr int max_length = 1024;
	char m_data[max_length];
	message_callback m_mcb;
};

class server
{
public:
	server(boost::asio::io_service& io_service, short port, message_callback mcb)
		: m_io_service(io_service)
		, m_acceptor(io_service, tcp::endpoint(tcp::v4(), port))
		, m_mcb(mcb)
	{
		start_accept();
	}

private:
	void start_accept()
	{
		session* new_session = new session(m_io_service, m_mcb);
		m_acceptor.async_accept(new_session->socket(),
			boost::bind(&server::handle_accept, this, new_session,
			boost::asio::placeholders::error));
	}

	void handle_accept(session* new_session, const boost::system::error_code& error)
	{
		if (!error)
			new_session->start();
		else
			delete new_session;

		start_accept();
	}

	boost::asio::io_service& m_io_service;
	tcp::acceptor m_acceptor;
	message_callback m_mcb;
};


