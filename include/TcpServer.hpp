#pragma once

#include <stdint.h>
#include <map>
#include <memory>

#include "PeerMessage.hpp"
#include "TcpConnection.hpp"
#include "TcpAcceptor.hpp"
#include "LooperPool.hpp"

using namespace std;

namespace netio {


typedef shared_ptr<TcpAcceptor> SpTcpAcceptor;
typedef shared_ptr<TcpConnection> SpTcpConnection;

class TcpServer {
  typedef function<void(SpTcpConnection)> NewConnectionHandler;
  
 public:
  // port to listen
  TcpServer(uint16_t port, int threads);
  ~TcpServer();

  void startWork();
  void stopWork();

  void setNewConnectionHandler(const NewConnectionHandler& handler) {
    _newConnHandler = handler;
  }

 private:
  // connection handler for TcpAcceptor
  void OnNewConnection(int fd, const InetAddr& addr);

  // tag for log
  static const char* LOG_TAG;

  // looper pool
  LooperPool<MultiplexLooper> _loopPool;

  // acceptor
  TcpAcceptor _acceptor;

  // callbacks for code that use this class
  NewConnectionHandler _newConnHandler;
};

}

