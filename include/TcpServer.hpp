#pragma once

#include <stdint.h>
#include <map>
#include <memory>
#include <set>

#include "TcpConnection.hpp"
#include "TcpAcceptor.hpp"
#include "LooperPool.hpp"

using namespace std;

namespace netio {

class TcpServer {
  typedef shared_ptr<TcpAcceptor> SpTcpAcceptor;
  typedef shared_ptr<LooperPool<MultiplexLooper> > SpLooperPool;
  typedef function<void(SpTcpConnection&)> NewConnectionHandler;
 public:
  // port to listen
  explicit TcpServer(uint16_t port, SpLooperPool loopPool);
  ~TcpServer();

  void startWork();
  void stopWork();

  void setConnectionHandler(const NewConnectionHandler& handler) {
    if(handler) {
      _newConnHandler = handler;      
    } else {
      _newConnHandler = std::bind(&TcpServer::dummyConnectionHandler, this, placeholders::_1);
    }
  }

  void setConnectionHandler(NewConnectionHandler&& handler) {
    if(handler) {
      _newConnHandler = std::move(handler);      
    } else {
      _newConnHandler = std::bind(&TcpServer::dummyConnectionHandler, this, placeholders::_1);      
    }
  }

  void removeConnection(const SpTcpConnection& connection) {
    _mainLooper->postRunnable(bind(&TcpServer::removeConnInLoop, this, connection));
  }

 private:
  void dummyConnectionHandler(const SpTcpConnection& conn) {
    LOGW("ts", "%s new connection, dummy", conn->strInfo());
  }
  void dummyMessageHandler(SpTcpConnection conn, SpVecBuffer& buffer) {
    LOGW("ts", "%s recv message, bufleft=%d, dummy", conn->strInfo(), buffer->readableSize());
  }
  void dummyResetConnectionHandler(const SpTcpConnection& conn, int cause) {
    LOGW("ts", "%s close connection, dummy", conn->strInfo());    
  }
  // connection handler for TcpAcceptor
  void onNewConnection(int fd, const InetAddr& addr);
  // remove connection in looper
  void removeConnInLoop(SpTcpConnection& conn) {
    conn->detach();
    int n = _connSet.erase(conn);
    if(UNLIKELY(n != 1)) {
      COGW("%s remove connection failed, conn=%s", __func__, conn->strInfo());
    }
  }

  // tag for log
  static const char* LOG_TAG;
  // looper pool
  SpLooperPool _loopPool;
  // manage looper
  MultiplexLooper* _mainLooper;
  // acceptor
  TcpAcceptor _acceptor;
  // TcpServer just hold TcpConnection, not for indexing.
  set<SpTcpConnection> _connSet;
  // callbacks for client code
  NewConnectionHandler _newConnHandler;
};

}











