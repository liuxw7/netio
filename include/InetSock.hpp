#pragma once

#include <netinet/in.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <sys/uio.h>

#include "InetAddr.hpp"

namespace netio {

class InetSock {
 public:
  InetSock(int fd);
  virtual ~InetSock();
  
  void enableReuseAddr(bool enable);
  void enableReusePort(bool enable);

  inline int setRecvBufSize(int size);
  inline int setSendBufSize(int size);

  inline int setSendTimeout(int msec);
  inline int setRecvTimeout(int msec);

  inline ssize_t sendmsg(const struct msghdr& msg, int flags);
  inline ssize_t recvmsg(struct msghdr& msg, int flags);

  int bind(const InetAddr& addr);
  int bind(const struct sockaddr_in& addr);
  int bind(uint16_t port);

 protected:
  int _fd;
};

class StreamSocket : public InetSock {
 public:
  StreamSocket(uint16_t port);
  StreamSocket(const struct sockaddr_in& sockaddr);
  inline int setKeepAlive(bool enable);
  inline ssize_t send(const void* buf, size_t len, int flags);
  inline ssize_t recv(void* buf, size_t len, int flags);
  inline ssize_t writev(const struct iovec* iov, int iovcnt);
  inline ssize_t readv(const struct iovec* iov, int iovcnt);
}

/**
 * Tcp client socket
 *
 * 
 */
class Socket : public StreamSocket {
 public:
  inline int connect(const struct sockaddr_in& remote);
};

/**
 * Tcp Server socket 
 */
class ServerSocket : public StreamSocket {
 public:
  inline int listen(int backlog);
  inline int accept(const struct sockaddr_in& clientaddr);
};

/**
 * Udp socket
 */
class DGramSocket : public InetSock {
 public:
  DGramSocket(uint16_t port);
  DGramSocket(const struct sockaddr_in& addr);

  inline ssize_t recvfrom(void* buf, size_t len,int flags, struct sockaddr_in& addr);
  inline ssize_t sendto(const void* buf, size_t len, int flags, const struct sockaddr_in& addr);
};

}
