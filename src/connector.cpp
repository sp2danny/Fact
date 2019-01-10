
#include "connector.h"

using boost::asio::ip::tcp;

// ----------------------------------------------------------------------------

// ***************
// *** session ***
// ***************

session::session(boost::asio::io_service& io_service, message_callback mcb)
	: m_socket(io_service)
	, m_mcb(mcb)
{
}

tcp::socket& session::socket()
{
	return m_socket;
}

void session::start()
{
	m_socket.async_read_some(boost::asio::buffer(m_data, max_length),
		boost::bind(&session::handle_read, this,
		boost::asio::placeholders::error,
		boost::asio::placeholders::bytes_transferred));
}

void session::handle_read(const boost::system::error_code& error, std::size_t bytes_transferred)
{
	if (!error)
	{
		std::string msg(m_data, m_data+bytes_transferred);
		std::string ret = m_mcb(msg);
		char buffer[max_length] = {0};
		strcpy(buffer, ret.c_str());
		boost::asio::async_write(m_socket,
			boost::asio::buffer(buffer, max_length),
			boost::bind(&session::handle_write, this,
			boost::asio::placeholders::error));
	} else {
		delete this;
	}
}

void session::handle_write(const boost::system::error_code& error)
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

// ----------------------------------------------------------------------------

// **************
// *** server ***
// **************

server::server(boost::asio::io_service& io_service, short port, message_callback mcb)
	: m_io_service(io_service)
	, m_acceptor(io_service, tcp::endpoint(tcp::v4(), port))
	, m_mcb(mcb)
{
	start_accept();
}

void server::start_accept()
{
	session* new_session = new session(m_io_service, m_mcb);
	m_acceptor.async_accept(new_session->socket(),
		boost::bind(&server::handle_accept, this, new_session,
		boost::asio::placeholders::error));
}

void server::handle_accept(session* new_session, const boost::system::error_code& error)
{
	if (!error)
		new_session->start();
	else
		delete new_session;

	start_accept();
}



