#pragma once

#include "PeerMessage.hpp"
#include "Netpack.hpp"
#include "Connection.hpp"
#include "Channel.hpp"
#include "InetSock.hpp"
#include "InetAddr.hpp"

namespace netio {

/**
 * 
 */
template <typename NP=FieldLenNetpack<GenericLenFieldHeader> >
class TcpConnection : public Connection {
 public:
  /**
   * Constructor usually for accepted connection.
   */
  explicit TcpConnection(int fd) :
      _sock(fd)
  {}

  /**
   * Constructor usually for client connection.
   */
  TcpConnection(uint16_t port) :
      _sock(port)
  {}

  int getFd() const {
    return _sock.getFd();
  }


  InetAddr getLocalAddr() const {
    struct sockaddr_in addr;
    bzero(&addr, sizeof(addr));
  
    socklen_t addrlen = static_cast<socklen_t>(sizeof(addr));
    int ret = ::getsockname(_sock.getFd(), SOCKADDR_CAST(&addr), &addrlen);
    return InetAddr(addr);
  }

  InetAddr getPeerAddr() const {
    struct sockaddr_in addr;
    bzero(&addr, sizeof(addr));
  
    socklen_t addrlen = static_cast<socklen_t>(sizeof(addr));
    int ret = ::getpeername(_sock.getFd(), SOCKADDR_CAST(&addr), &addrlen);
    return InetAddr(addr);
  }

 private:
  Channel<NP> _channel;
  StreamSocket _sock;
};

}

















