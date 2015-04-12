#pragma once

#include <netinet/in.h>
#include <string>

namespace netio
{

#define SOCKADDR_CAST(addr) (struct sockaddr *)(addr)

class InetAddr {
 public:
  InetAddr(const struct sockaddr_in& addr);
  InetAddr(std::string ip, uint16_t port);
  explicit InetAddr(uint32_t ip, uint16_t port);
  explicit InetAddr(uint16_t port);
  
  std::string strIp() const;
  std::string strIpPort() const;
  uint32_t ip() const;
  uint16_t port() const;

  uint32_t netEndianIp() const;
  uint32_t netEndianPort() const;

  static bool resolve(const std::string& host, InetAddr& addr);

  const struct sockaddr_in& getSockAddr() const {
    return _sockaddr;
  }
  
 private:
  struct sockaddr_in _sockaddr;
};

}















