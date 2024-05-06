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

class ProxyServer
{
public:
  ProxyServer(const ProxyConfig &config) : config_(config) {}

  void start()
  {
    // Create I/O service object
    boost::asio::io_service io_service;

    // Create acceptor object with specified port from config
    boost::asio::ip::tcp::acceptor acceptor(io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), config_.port));

    while (true)
    {
      // Create socket object to accept incoming connections
      boost::asio::ip::tcp::socket socket(io_service);

      // Wait for a new connection
      acceptor.accept(socket);

      // Handle connection in a separate thread (optional)
      std::thread([this, &socket]()
                  { handleConnection(socket); })
          .detach();
    }
  }

private:
  ProxyConfig config_;

  void handleConnection(boost::asio::ip::tcp::socket &clientSocket)
  {
    try
    {
      // Read the request from the client
      std::string request;
      boost::asio::read_until(clientSocket, boost::asio::dynamic_buffer(request), "\r\n\r\n");

      // Forward the request to the destination server
      std::string response = forwardRequest(request);

      // Send the response back to the client
      boost::asio::write(clientSocket, boost::asio::buffer(response));
    }
    catch (std::exception &e)
    {
      std::cerr << "Exception in handleConnection: " << e.what() << std::endl;
    }
  }

  std::string forwardRequest(const std::string &request)
  {
    try
    {
      // Basic parsing to extract the destination server's hostname and port
      std::string destinationHost, destinationPort;
      std::istringstream iss(request);
      std::string method, url, protocol;
      std::getline(iss, method, ' '); // Skip the method
      std::getline(iss, url, ' ');    // Extract the URL
      std::getline(iss, protocol);    // Skip the protocol

      // Extract hostname and port from the URL
      std::size_t pos = url.find("://");
      if (pos != std::string::npos)
      {
        url.erase(0, pos + 3); // Remove the protocol part
      }
      pos = url.find('/');
      if (pos != std::string::npos)
      {
        std::string hostAndPort = url.substr(0, pos); // Extract hostname and port
        pos = hostAndPort.find(':');
        if (pos != std::string::npos)
        {
          destinationHost = hostAndPort.substr(0, pos);
          destinationPort = hostAndPort.substr(pos + 1);
        }
        else
        {
          destinationHost = hostAndPort;
          destinationPort = "80";
        }
      }

      // Create a socket for the destination server
      boost::asio::io_service io_service;
      boost::asio::ip::tcp::socket destinationSocket(io_service);

      // Resolve the destination server's address
      boost::asio::ip::tcp::resolver resolver(io_service);
      boost::asio::ip::tcp::resolver::query query(destinationHost, destinationPort);
      boost::asio::ip::tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

      // Connect to the destination server
      boost::asio::connect(destinationSocket, endpoint_iterator);

      // Send the request to the destination server
      boost::asio::write(destinationSocket, boost::asio::buffer(request));

      // Read the response from the destination server
      std::string response;
      boost::asio::read_until(destinationSocket, boost::asio::dynamic_buffer(response), "\r\n\r\n");

      // Close the connection to the destination server
      destinationSocket.close();

      return response;
    }
    catch (std::exception &e)
    {
      std::cerr << "Exception in forwardRequest: " << e.what() << std::endl;
      return "";
    }
  }
};