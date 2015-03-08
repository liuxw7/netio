
#include <unistd.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <strings.h>
#include <unistd.h>
#include <fcntl.h>

#include "Utils.hpp"
#include "InetSock.hpp"
#include "Endian.hpp"


namespace netio {

#define ENSURE_FD(fd)   ASSERT(fd >= 0)
#define ENSURE_RET(ret)  ASSERT(-1 != ret)

InetSock::InetSock(int fd) : _fd(fd) {}
InetSock::~InetSock() {}

InetAddr InetSock::getLocalAddr() const {
  struct sockaddr_in addr;
  bzero(&addr, sizeof(addr));
  
  socklen_t addrlen = static_cast<socklen_t>(sizeof(addr));
  int ret = ::getsockname(_fd, SOCKADDR_CAST(&addr), &addrlen);
  return InetAddr(addr);
}

InetAddr InetSock::getPeerAddr() const {
  struct sockaddr_in addr;
  bzero(&addr, sizeof(addr));
  
  socklen_t addrlen = static_cast<socklen_t>(sizeof(addr));
  int ret = ::getpeername(_fd, SOCKADDR_CAST(&addr), &addrlen);
  return InetAddr(addr);
}

void InetSock::enableReuseAddr(bool enable) {
  int optval = enable ? 1 : 0;
  ::setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &optval, static_cast<socklen_t>(sizeof optval));
}

void InetSock::enableReusePort(bool enable) {
#ifndef SO_REUSEPORT
  ASSERT(false == enable);
#endif
  int optval = enable ? 1 : 0;
  ::setsockopt(_fd, SOL_SOCKET, SO_REUSEPORT, &optval, static_cast<socklen_t>(sizeof optval));
}


void InetSock::setNonblocking(bool enable) {
  int flags;
  CHKRET(flags = fcntl(_fd, F_GETFL, 0));

  if(enable) {
    flags |= O_NONBLOCK;
  } else {
    flags &= (~O_NONBLOCK);
  }
  
  CHKRET(fcntl(_fd, F_SETFL, flags));
}

int InetSock::setRecvBufSize(int size) {
  return setsockopt(_fd, SOL_SOCKET, SO_RCVBUF, &size, static_cast<socklen_t>(sizeof(int)));
}

int InetSock::setSendBufSize(int size) {
  return setsockopt(_fd, SOL_SOCKET, SO_SNDBUF, &size, static_cast<socklen_t>(sizeof(int)));
}

int InetSock::setSendTimeout(int msec) {
  return setsockopt(_fd, SOL_SOCKET, SO_SNDTIMEO, &msec, static_cast<socklen_t>(sizeof(int)));
}

int InetSock::setRecvTimeout(int msec) {
  return setsockopt(_fd, SOL_SOCKET, SO_RCVTIMEO, &msec, static_cast<socklen_t>(sizeof(int)));
}

int InetSock::getSocketError() const {
  int optval;
  socklen_t optlen = static_cast<socklen_t>(sizeof(optval));

  if (::getsockopt(_fd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0) {
    return errno;
  } else {
    return optval;
  }
}

int InetSock::bind(const InetAddr& addr) {
  return ::bind(_fd, (const struct sockaddr *)&addr.getSockAddr(),
                static_cast<socklen_t>(sizeof(decltype(addr.getSockAddr()))));
}

int InetSock::bind(const struct sockaddr_in& addr) {
  return ::bind(_fd, (const struct sockaddr*)&addr, static_cast<socklen_t>(sizeof(struct sockaddr_in)));
}

int InetSock::bind(uint16_t port) {
  struct sockaddr_in addr;

  bzero(&addr, sizeof(struct sockaddr_in));
  addr.sin_family = AF_INET;
  addr.sin_port = Endian::hton16(port);
  addr.sin_addr.s_addr = INADDR_ANY;

  return ::bind(_fd, (const struct sockaddr *)&addr, static_cast<socklen_t>(sizeof(struct sockaddr_in)));
}

StreamSocket::StreamSocket(int fd) : InetSock(fd) {}

StreamSocket::StreamSocket(uint16_t port) : InetSock(socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) {
  ENSURE_FD(_fd);
  int ret = bind(port);
  ENSURE_RET(ret);
}

StreamSocket::StreamSocket(const struct sockaddr_in& sockaddr) : InetSock(socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)){
  ENSURE_FD(_fd);
  int ret = bind(sockaddr);
  ENSURE_RET(ret);
}

int StreamSocket::setKeepAlive(bool enable) {
  int optval = enable ? 1 : 0;
  return ::setsockopt(_fd, SOL_SOCKET, SO_KEEPALIVE, &optval, static_cast<socklen_t>(sizeof optval));
}

int Socket::connect(const struct sockaddr_in& remote) {
  return ::connect(_fd, (const struct sockaddr*)&remote, static_cast<socklen_t>(sizeof(struct sockaddr_in)));
}

int ServerSocket::listen(int backlog) {
  return ::listen(_fd, backlog);
}

int ServerSocket::accept(const struct sockaddr_in& clientaddr) {
  socklen_t addrlen = static_cast<socklen_t>(sizeof(struct sockaddr_in));
  return ::accept(_fd, (struct sockaddr*)&clientaddr, &addrlen);
}

DGramSocket::DGramSocket(uint16_t port) : InetSock(socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) {
  ENSURE_FD(_fd);
  int ret = bind(port);
  ENSURE_RET(ret);
}

DGramSocket::DGramSocket(const struct sockaddr_in& addr) : InetSock(socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) {
  ENSURE_FD(_fd);
  int ret = bind(addr);
  ENSURE_RET(ret);
}

}
















