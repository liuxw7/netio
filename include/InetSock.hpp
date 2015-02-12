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

  int setRecvBufSize(int size);
  int setSendBufSize(int size);

  int setSendTimeout(int msec);
  int setRecvTimeout(int msec);

  ssize_t sendmsg(const struct msghdr& msg, int flags) {
    
  }
  ssize_t recvmsg(struct msghdr& msg, int flags) {
    
  }

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
  int setKeepAlive(bool enable);
  ssize_t send(const void* buf, size_t len, int flags) {
    
  }
  ssize_t recv(void* buf, size_t len, int flags) {
    
  }
  ssize_t writev(const struct iovec* iov, int iovcnt) {
    
  }
  ssize_t readv(const struct iovec* iov, int iovcnt) {
    
  }
}

/**
 * Tcp client socket
 *
 * 
 */
class Socket : public StreamSocket {
 public:
  int connect(const struct sockaddr_in& remote);
};

/**
 * Tcp Server socket 
 */
class ServerSocket : public StreamSocket {
 public:
  int listen(int backlog);
  int accept(const struct sockaddr_in& clientaddr);
};

/**
 * Udp socket
 */
class DGramSocket : public InetSock {
 public:
  DGramSocket(uint16_t port);
  DGramSocket(const struct sockaddr_in& addr);

  ssize_t recvfrom(void* buf, size_t len,int flags, struct sockaddr_in& addr) {
    
  }
  ssize_t sendto(const void* buf, size_t len, int flags, const struct sockaddr_in& addr) {
    
  }
};

}
