#pragma once

#include "Channel.hpp"
#include "InetSock.hpp"
#include "MultiplexLooper.hpp"

namespace netio {

class TcpAcceptor {
 public:
  TcpAcceptor(MultiplexLooper* looper, uint16_t port);
  ~TcpAcceptor();
  
  void handleRead();
  void attach();
  void detach();
 private:
  ServerSocket _sock;
  Channel _channel;
};

}














