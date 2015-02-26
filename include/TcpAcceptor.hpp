#pragma once

#include <functional>
#include <memory>

#include "Channel.hpp"
#include "InetAddr.hpp"
#include "InetSock.hpp"
#include "MultiplexLooper.hpp"

namespace netio {

class TcpAcceptor {
  typedef function<void(int fd, const InetAddr&)> OnNewConnection;
 public:
  TcpAcceptor(MultiplexLooper* looper, uint16_t port);
  ~TcpAcceptor();

  void setNewConnCallback(const OnNewConnection& onNewConn) {
    _onNewConn = onNewConn;
  }
#ifdef __GXX_EXPERIMENTAL_CXX0X__
  void setNewConnCallback(const OnNewConnection&& onNewConn) {
    _onNewConn = std::move(onNewConn);
  }
#endif
  
  void handleRead();
  void attach();
  void detach();
 private:
  ServerSocket _sock;
  Channel _channel;
  // On new connection callback.
  OnNewConnection _onNewConn;
};



}











