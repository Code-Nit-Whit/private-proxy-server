#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include "../include/ProxyServer.hpp" 
#include "../include/ProxyConfig.hpp"

bool loadConfig(const std::string& filename, ProxyConfig& config) {
  std::ifstream configFile(filename);
  if (!configFile.is_open()) {
    std::cerr << "Error opening configuration file: " << filename << std::endl;
    return false;
  }

  std::string line;
  while (std::getline(configFile, line)) {
    std::istringstream iss(line);
    std::string key, value;
    if (std::getline(iss, key, '=') && std::getline(iss, value)) {
      if (key == "port") {
        config.port = std::stoi(value);
      }
      // Add logic to handle other configuration options (username, password)
    }
  }

  configFile.close();
  return true;
}

int main() {
  ProxyConfig config;
  if (!loadConfig("proxy.conf", config)) {
    return 1;
  }

  ProxyServer server(config);
  server.start();

  return 0;
}
