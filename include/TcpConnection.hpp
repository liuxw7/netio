#pragma once

#include <netinet/in.h>

#include "MultiplexLooper.hpp"
#include "Channel.hpp"
#include "InetAddr.hpp"
#include "InetSock.hpp"

namespace netio {

class TcpConnection {
 public:
  TcpConnection(MultiplexLooper* looper, int fd, const struct sockaddr_in& addr);
  ~TcpConnection();

  void handleRead();
  void handleWrite();
  void attach();
  void detach();
 private:
  InetAddr _peerAddr;
  StreamSocket _sock;
  Channel _channel;
};

}

















