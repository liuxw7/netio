
#include <unistd.h>
#include "InetSock.hpp"

namespace netio {

InetSock::InetSock(int fd) : _fd(fd) {}

InetSock::~InetSock() {
  if(_fd >= 0) {
    close(_fd);
  }
}

void InetSock::enableReuseAddr(bool enable) {
  int ret, on = enable ? 1 : 0;
  ret = setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &on, static_cast<socklen_t>(sizeof(on)));
}

void InetSock::enableReusePort(bool enable) {
  int ret, on = enable ? 1 : 0;
  ret = setsockopt(_fd, SOL_SOCKET, SO_REUSEPORT, &on, static_cast<socklen_t>(sizeof(on)));
}

int InetSock::setRecvBufSize(int size) {
  int ret;
  ret = setsockopt(_fd, SOL_SOCKET, SO_RCVBUF, &size, static_cast<socklen_t>(sizeof(size)));
}

int InetSock::setSendBufSize(int size) {
  int ret;
  ret = setsockopt(_fd, SOL_SOCKET, SO_SNDBUF, &size, static_cast<socklen_t>(sizeof(size)));
}


int InetSock::setSendTimeout(int msec) {
  
}

int InetSock::setRecvTimeout(int msec) {
  
}

ssize_t InetSock::sendmsg(const struct msghdr& msg, int flags) {
  
}

ssize_t InetSock::recvmsg(struct msghdr& msg, int flags) {
  
}

int InetSock::bind(const struct sockaddr_in& addr) {
  
}

int InetSock::bind(uint16_t port) {
  
}

StreamSocket::StreamSocket(uint16_t port) {
  
}

StreamSocket::StreamSocket(const struct sockaddr_in& sockaddr) {
  
}

int StreamSocket::setKeepAlive(bool enable) {
  
}

ssize_t StreamSocket::send(const void* buf, size_t len, int flags) {
  
}

ssize_t StreamSocket::recv(void* buf, size_t len, int flags) {
  
}

ssize_t StreamSocket::writev(const struct iovec* iov, int iovcnt) {
  
}

ssize_t StreamSocket::readv(const struct iovec* iov, int iovcnt) {
  
}

int Socket::connect(const struct sockaddr_in& remote) {
  
}

int ServerSocket::listen(int backlog) {

}

int ServerSocket::accept(const struct sockaddr_in& clientaddr) {
  
}

DGramSocket::DGramSocket(uint16_t port) {
}

DGramSocket::DGramSocket(const struct sockaddr_in& addr) {
}

ssize_t DGramSocket::recvfrom(void* buf, size_t len,int flags, struct sockaddr_in& addr) {
  
}

ssize_t DGramSocket::sendto(const void* buf, size_t len, int flags, const struct sockaddr_in& addr) {
  
}

}
