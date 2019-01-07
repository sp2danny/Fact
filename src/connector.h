
#pragma once

#include <string>

#include <boost/asio.hpp>

using boost::asio::ip::tcp;

class tcp_connection
{
public:

  tcp_connection(boost::asio::io_service&);

private:
  boost::asio::ip::tcp::socket m_socket;
  std::string m_message;
};

class tcp_server
{
public:
  tcp_server(boost::asio::io_service&);

private:


  boost::asio::ip::tcp::acceptor m_acceptor;
};



