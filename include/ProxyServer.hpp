#ifndef PROXYSERVER_HPP
#define PROXYSERVER_HPP

#include <cstddef> 
#include <boost/asio.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>
#include <iostream>
#include <string>
#include <thread>
#include <sstream> 
#include "ProxyConfig.hpp"

class ProxyServer {
public:
  ProxyServer(const ProxyConfig& config);
  void start();

private:
  ProxyConfig config_;
  void handleConnection(boost::asio::ip::tcp::socket& socket);
  std::string forwardRequest(const std::string& request); 
};

#endif // PROXYSERVER_HPP
