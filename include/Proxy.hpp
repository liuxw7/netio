#pragma once

#include <memory>

#include "TcpServer.hpp"
#include "MultiplexLooper.hpp"
#include "LooperPool.hpp"
#include "NetPackDispatcher.hpp"
#include "FieldLenNetPack.hpp"
#include "TcpConnection.hpp"
#include "Session.hpp"
#include "TimerWrap.hpp"
#include "HashedWheelTimer.hpp"
#include "Logger.hpp"

using namespace netio;
using namespace std;

class TcpProxy {
  typedef FLNPack::MsgType::CmdType CmdType;
  typedef shared_ptr<LooperPool<MultiplexLooper> > SpLooperPool;
  typedef NetPackDispatcher<FLNPack, TcpConnection> TcpDispatcher;
  typedef shared_ptr<TcpSession> SpSession;  
  enum { TimerInterval = 100 };
 public:
  
  TcpProxy(const SpLooperPool& loopers, uint16_t lport, uint32_t expired) :
      _loopPool(loopers),
      _server(lport, _loopPool), 
      _dispatcher(),
      _sm(),
      _timer(_loopPool->getLooper(), 100, expired / TimerInterval)
  {
    _server.setNewConnectionHandler(std::bind(&TcpProxy::onNewConnection, this, std::placeholders::_1, std::placeholders::_2));
  }
  
  TcpProxy(size_t threadCount, uint16_t lport, uint32_t expired) :
      _loopPool(new LooperPool<MultiplexLooper>(threadCount)),
      _server(lport, _loopPool),
      _dispatcher(),
      _sm(),
      _timer(_loopPool->getLooper(), 100, expired / TimerInterval)  
  {
    _server.setNewConnectionHandler(std::bind(&TcpProxy::onNewConnection, this, std::placeholders::_1, std::placeholders::_2));
  }

  void startWork() {
    //   timerWrapper.addTimeout(func, 5000);
    _server.startWork();
  }
  
  void stopWork() {
    _server.stopWork();
  }

  void registerHandler(const TcpDispatcher::Handler& handler) {
    _dispatcher.registerHandler(handler);
  }

  void registerHandler(TcpDispatcher::Handler&& handler) {
    _dispatcher.registerHandler(std::move(handler));
  }

  void registerHandler(CmdType& cmd, const TcpDispatcher::Handler& callback) {
    _dispatcher.registerHandler(cmd, callback);
  }
  
  void registerHandler(CmdType& cmd, TcpDispatcher::Handler&& callback) {
    _dispatcher.registerHandler(cmd, std::move(callback));
  }

  void addSession(const SpSession& session) {
    _sm.addSession(session);
  }

  void addSession(SpSession&& session) {
    _sm.addSession(std::move(session));
  }

  void touchSession(uint64_t cid) {
    _sm.touchSessionByCid(cid);
  }

  void touchSession(uint64_t cid, uint64_t ts) {
    _sm.touchSessionByCid(cid, ts);
  }
  
 private:
  void onNewConnection(int connId, SpTcpConnection& connection) {
    COGI("TcpProxy connection establish, remoteaddr=%s ", connection->getPeerAddr().strIpPort().c_str());
    connection->setNewMessageHandler(std::bind(&TcpDispatcher::dispatch, &_dispatcher, std::placeholders::_1, std::placeholders::_2));
    connection->attach();
  }
  
  SpLooperPool _loopPool;
  TcpServer _server;
  TcpDispatcher _dispatcher;
  SessionManager _sm;
  TimerWrap<HashedWheelTimer> _timer;
};



