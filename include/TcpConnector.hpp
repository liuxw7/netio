#pragma once

#include <functional>
#include <memory>

#include "Channel.hpp"
#include "InetAddr.hpp"
#include "InetSock.hpp"
#include "MultiplexLooper.hpp"

namespace netio {

class TcpConnector {
  typedef function<void(int fd, const InetAddr&)> OnNewConnection;
 public:
  TcpConnector(MultiplexLooper* looper, uint16_t localPort, const InetAddr& remoteAddr);
  ~TcpConnector();

  void setNewConnCallback(const OnNewConnection& onNewConn) {
    _onNewConn = onNewConn;
  }
#ifdef __GXX_EXPERIMENTAL_CXX0X__
  void setNewConnCallback(const OnNewConnection&& onNewConn) {
    _onNewConn = std::move(onNewConn);
  }
#endif

  void connect() {
    _channel.getLooper()->postRunnable(bind(&TcpConnector::connectInternal, this));
  }
  
  void handleRead();
  void attach();
  void detach();
 private:
  void connectInternal();
  Socket _sock;
  InetAddr _remoteAddr;
  Channel _channel;
  // On new connection callback.
  OnNewConnection _onNewConn;
};

}












