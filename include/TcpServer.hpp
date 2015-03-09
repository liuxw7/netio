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
typedef shared_ptr<TcpConnection> SpTcpConnection;
typedef shared_ptr<LooperPool<MultiplexLooper> > SpLooperPool;

class TcpServer {
  typedef function<void(int, SpTcpConnection)> NewConnectionHandler;
  
 public:
  // port to listen
  TcpServer(uint16_t port, SpLooperPool loopPool);
  ~TcpServer();

  void startWork();
  void stopWork();

  static int connectionHashCode(SpTcpConnection& connection) {
    return connection->getFd();
  }

  void setNewConnectionHandler(const NewConnectionHandler& handler) {
    _newConnHandler = handler;
  }

  void removeConnection(int hashCode) {
    _mainLooper->postRunnable(bind(&TcpServer::removeConnByKeyInLoop, this, hashCode));
  }

  void removeConnection(SpTcpConnection& connection) {
    _mainLooper->postRunnable(bind(&TcpServer::removeConnInLoop, this, connection));
  }
  
  SpTcpConnection& getConnection(int hashCode) {
    return _connMap[hashCode];
  }

 private:
  // tag for log
  static const char* LOG_TAG;

  // callbacks for client code
  NewConnectionHandler _newConnHandler;
  
  // connection handler for TcpAcceptor
  void OnNewConnection(int fd, const InetAddr& addr);
  // remove connection in looper
  void removeConnInLoop(SpTcpConnection& conn) {
    int hashCode = TcpServer::connectionHashCode(conn);
    COGI("%s, hashKey=%d", __func__, hashCode);
    conn->detach();
    
    int n = _connMap.erase(hashCode);
    if(UNLIKELY(n != 1)) {
      COGW("%s remove connection failed, key=%d", __func__, TcpServer::connectionHashCode(conn));
    }
  }
  void removeConnByKeyInLoop(int hashCode) {
    COGI("%s, hashKey=%d", __func__, hashCode);
    SpTcpConnection& conn  = _connMap[hashCode];

    if(LIKELY(nullptr != conn)) {
      _connMap.erase(hashCode);
      conn->detach();
    } else {
      COGW("%s connection not found, key=%d", __func__, hashCode);
    }
    
  }
  // looper pool
  SpLooperPool _loopPool;
  // manage looper
  MultiplexLooper* _mainLooper;
  // acceptor
  TcpAcceptor _acceptor;
 
  // Connections map, we use fd for map key
  map<int, SpTcpConnection> _connMap;
};

}

