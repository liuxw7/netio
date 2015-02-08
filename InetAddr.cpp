
#include <strings.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "InetAddr.hpp"
#include "Endian.hpp"

namespace netio {

InetAddr::InetAddr(const struct sockaddr_in& addr) : _sockaddr(addr) {}

InetAddr::InetAddr(std::string ip, uint16_t port) {
  bzero(&_sockaddr, sizeof(struct sockaddr_in));

  _sockaddr.sin_family = AF_INET;
  _sockaddr.sin_port = Endian::hton16(port);
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
  _sockaddr.sin_port = Endian::hton16(port);
}
  
std::string InetAddr::strIp() const {
  char ipstr[16] = {0};
  inet_ntop(AF_INET, (void*)&_sockaddr.sin_addr, ipstr, sizeof(ipstr));
  return ipstr;
}

std::string InetAddr::strIpPort() const {
  char ipstr[16] = {0};
  char ipportstr[22] = {0};

  inet_ntop(AF_INET, (void*)&_sockaddr.sin_addr, ipstr, sizeof(ipstr));
  snprintf(ipportstr, sizeof(ipportstr), "%s:%u", ipstr, Endian::ntoh16(_sockaddr.sin_port));
  return ipportstr;
}

uint32_t InetAddr::ip() const {
  return Endian::ntoh32(_sockaddr.sin_addr.s_addr);
}
uint16_t InetAddr::port() const {
  return Endian::ntoh16(_sockaddr.sin_port);
}

uint32_t InetAddr::netEndianIp() const {
  return _sockaddr.sin_addr.s_addr;
}

uint32_t InetAddr::netEndianPort() const {
  return _sockaddr.sin_port;
}

static __thread char __resolve_buf[8 * 1024];

bool InetAddr::resolve(const std::string& host, InetAddr& addr) {
  struct hostent ent;
  struct hostent *pent = nullptr;
  int henterr = 0;
  
  bzero(&ent, sizeof(struct hostent));

  int ret = gethostbyname_r(host.c_str(), &ent, __resolve_buf, sizeof(__resolve_buf), &pent, &henterr);
  if(0 == ret && pent != nullptr) {
    addr._sockaddr.sin_addr = *reinterpret_cast<struct in_addr*>(pent->h_addr);
    return true;
  } else {
    return false;
  }
}

}
