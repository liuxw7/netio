#pragma once

#include <map>
#include <time.h>
#include <memory>
#include <functional>

#include "TcpServer.hpp"
#include "TcpConnection.hpp"
#include "Dispatcher.hpp"

namespace netio {

class UserSession {
  typedef shared_ptr<TcpConnection> SpTcpConnection;
 public:
  UserSession(int id, time_t createTime, SpTcpConnection connection) :
      _id(id),
      _createTime(createTime),
      _updateTime(createTime),
      _connection(connection),
      _seq(0)
  {}

  void touch(time_t updateTime) { _updateTime = updateTime; }

  uint32_t getId() const { return _id; }

  time_t createTime() const { return _createTime; }

  time_t updateTime() const { return _updateTime; }

  
 private:
  uint32_t _id; // session id is unique global for distribute system.
  time_t _createTime;
  time_t _updateTime;
  uint32_t _seq; // sequence number for generate push message sequence number.
  SpTcpConnection _connection;
};

class UserSessionMgr {
 public:
  void process(SpVecBuffer& buf, PMAddr& addr) {
    
  }
 private:
  map<int, UpUserSession> _usMap;
};

/**
 * 
 */
class TcpProxy {
  typedef shared_ptr<LooperPool<MultiplexLooper> > SpLooperPool;
  typedef unique_ptr<UserSession> UpUserSession;
 public:
  /**
   * @loopsers hold threads for process events.
   * @servPort for client connection.
   * @pushPort for inner module transport that aim for send data to client
   */
  explicit TcpProxy(SpLooperPool& loopers, uint16_t servPort, uint16_t pushPort);

  
  Dispatcher& getDispatcher() { return _dispatcher; }
 private:
  
  SpLooperPool _loopPool;
  UserSessionMgr _usMgr;
  TcpServer _server;
  Dispatcher<int> _dispatcher;
};

TcpProxy::TcpProxy(SpLooperPool& loopers, uint16_t servPort, uint16_t pushPort) :
    _loopPool(loopers),
    _server(servPort, loopers)
{
  _dispatcher.registerAnyHandler(std::bind(&UserSessionMgr::process, &_usMgr, std::placeholders::_1, std::placeholders::_2));
}

}


