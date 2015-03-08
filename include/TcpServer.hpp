#pragma once

#include <stdint.h>
#include <map>
#include <memory>

#include "PeerMessage.hpp"
#include "TcpConnection.hpp"
#include "TcpAcceptor.hpp"
#include "LooperPool.hpp"
#include "FieldLenNetPack.hpp"

using namespace std;

namespace netio {

typedef shared_ptr<TcpAcceptor> SpTcpAcceptor;
typedef shared_ptr<TcpConnection<GenFieldLenPack> > SpTcpConnection;

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

  void addConnection() {
    
  }

  void removeConnection() {
    
  }

 private:
  // tag for log
  static const char* LOG_TAG;
  
  // connection handler for TcpAcceptor
  void OnNewConnection(int fd, const InetAddr& addr);
  // looper pool
  LooperPool<MultiplexLooper> _loopPool;
  // acceptor
  TcpAcceptor _acceptor;
  // callbacks for code that use this class
  NewConnectionHandler _newConnHandler;


  // Connection map
  map<uint32_t, SpTcpConnection> _connMap;
  mutable mutex _mapMutex;
};

}

