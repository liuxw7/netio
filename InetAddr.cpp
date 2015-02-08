
#include <strings.h>
#include <arpa/inet.h>

#include "InetAddr.hpp"
#include "Endian.hpp"

namespace netio {

InetAddr::InetAddr(const struct sockaddr_in& addr) : _sockaddr(addr) {}

InetAddr::InetAddr(std::string ip, uint16_t port) {
  bzero(&_sockaddr, sizeof(struct sockaddr_in));

  _sockaddr.sin_family = AF_INET;
  _sockaddr.sin_port = Endian::hton16(port());
  inet_pton(AF_INET, ip.c_str(), &_sockaddr.sin_addr);
}

InetAddr::InetAddr(uint32_t ip, uint16_t port) {
  bzero(&_sockaddr, sizeof(struct sockaddr_in));
  
  _sockaddr.sin_family = AF_INET;
  _sockaddr.sin_addr.s_addr = Endian::hton32(ip);
  _sockaddr.sin_port = Endian::hton16(port);
}

InetAddr::InetAddr(uint16_t port) {
  bzero(&_sockaddr, sizeof(struct sockaddr_in));

  _sockaddr.sin_family = AF_INET;
  _sockaddr.sin_addr.s_addr = INADDR_ANY;
  _sockaddr.sin_port = Endian::hton16(port());
}
  
std::string InetAddr::strIp() const {
  inet_ntop(int __af, const void *restrict __cp, char *restrict __buf, socklen_t __len)
}

std::string InetAddr::strIpPort() const {
  
}

uint32_t InetAddr::ip() const {
  
}
uint16_t InetAddr::port() const {
  
}

uint32_t InetAddr::netEndianIp() const {
  
}

uint32_t InetAddr::netEndianPort() const {
  
}

bool InetAddr::resolve(const std::string& host, InetAddr& addr) {
  
}

}




















