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
  typedef function<void(SpTcpConnection&, SpVecBuffer&)> NewMessageHandler;
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

  void setMessageHandler(const NewMessageHandler& handler) {
    if(handler) {
      _newMsgHandler = handler;
    } else {
      _newMsgHandler = std::bind(&TcpServer::dummyMessageHandler, this, placeholders::_1, placeholders::_2);
    }
  }

  void setMessageHandler(NewMessageHandler&& handler) {
    if(handler) {
      _newMsgHandler = std::move(handler);
    } else {
      _newMsgHandler = std::bind(&TcpServer::dummyMessageHandler, this, placeholders::_1, placeholders::_2);
    }
  }

  void removeConnection(SpTcpConnection& connection) {
    _mainLooper->postRunnable(bind(&TcpServer::removeConnInLoop, this, connection));
  }

 private:
  void dummyConnectionHandler(SpTcpConnection& conn) {
    FOGI("tcpserver receive connection : %s", conn->strInfo());
  }
  void dummyMessageHandler(SpTcpConnection& conn, SpVecBuffer& buffer) {
    FOGW("tcpserver receive msg, no message handler");
  }
  // connection handler for TcpAcceptor
  void OnNewConnection(int fd, const InetAddr& addr);
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
  // callbacks for client code
  NewMessageHandler _newMsgHandler;
};

}











